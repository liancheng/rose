/*
 *  linux/kernel/acct.c
 *
 *  BSD Process Accounting for Linux
 *
 *  Author: Marco van Wieringen <mvw@planets.elm.net>
 *
 *  Some code based on ideas and code from:
 *  Thomas K. Dyas <tdyas@eden.rutgers.edu>
 *
 *  This file implements BSD-style process accounting. Whenever any
 *  process exits, an accounting record of type "struct acct" is
 *  written to the file specified with the acct() system call. It is
 *  up to user-level programs to do useful things with the accounting
 *  log. The kernel just provides the raw accounting information.
 *
 * (C) Copyright 1995 - 1997 Marco van Wieringen - ELM Consultancy B.V.
 *
 *  Plugged two leaks. 1) It didn't return acct_file into the free_filps if
 *  the file happened to be read-only. 2) If the accounting was suspended
 *  due to the lack of space it happily allowed to reopen it and completely
 *  lost the old acct_file. 3/10/98, Al Viro.
 *
 *  Now we silently close acct_file on attempt to reopen. Cleaned sys_acct().
 *  XTerms and EMACS are manifestations of pure evil. 21/10/98, AV.
 *
 *  Fixed a nasty interaction with with sys_umount(). If the accointing
 *  was suspeneded we failed to stop it on umount(). Messy.
 *  Another one: remount to readonly didn't stop accounting.
 *	Question: what should we do if we have CAP_SYS_ADMIN but not
 *  CAP_SYS_PACCT? Current code does the following: umount returns -EBUSY
 *  unless we are messing with the root. In that case we are getting a
 *  real mess with do_remount_sb(). 9/11/98, AV.
 *
 *  Fixed a bunch of races (and pair of leaks). Probably not the best way,
 *  but this one obviously doesn't introduce deadlocks. Later. BTW, found
 *  one race (and leak) in BSD implementation.
 *  OK, that's better. ANOTHER race and leak in BSD variant. There always
 *  is one more bug... 10/11/98, AV.
 *
 *	Oh, fsck... Oopsable SMP race in do_process_acct() - we must hold
 * ->mmap_sem to walk the vma list of current->mm. Nasty, since it leaks
 * a struct file opened for write. Fixed. 2/6/2000, AV.
 */

#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/acct.h>
#include <linux/capability.h>
#include <linux/file.h>
#include <linux/tty.h>
#include <linux/security.h>
#include <linux/vfs.h>
#include <linux/jiffies.h>
#include <linux/times.h>
#include <linux/syscalls.h>
#include <linux/mount.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <linux/blkdev.h> /* sector_div */

/*
 * These constants control the amount of freespace that suspend and
 * resume the process accounting system, and the time delay between
 * each check.
 * Turned into sysctl-controllable parameters. AV, 12/11/98
 */

int acct_parm[3] = {4, 2, 30};
#define RESUME		(acct_parm[0])	/* >foo% free space - resume */
#define SUSPEND		(acct_parm[1])	/* <foo% free space - suspend */
#define ACCT_TIMEOUT	(acct_parm[2])	/* foo second timeout between checks */

/*
 * External references and all of the globals.
 */
static void do_acct_process(struct file *);

/*
 * This structure is used so that all the data protected by lock
 * can be placed in the same cache line as the lock.  This primes
 * the cache line to have the data after getting the lock.
 */
struct acct_glbs {
	spinlock_t		lock;
	volatile int		active;
	volatile int		needcheck;
	struct file		*file;
	struct timer_list	timer;
};

static struct acct_glbs acct_globals __cacheline_aligned =
	{__SPIN_LOCK_UNLOCKED(acct_globals.lock)};

/*
 * Called whenever the timer says to check the free space.
 */
static void acct_timeout(unsigned long unused)
{
	acct_globals.needcheck = 1;
}

/*
 * Check the amount of free space and suspend/resume accordingly.
 */
static int check_free_space(struct file *file)
{
	struct kstatfs sbuf;
	int res;
	int act;
	sector_t resume;
	sector_t suspend;

	spin_lock(&acct_globals.lock);
	res = acct_globals.active;
	if (!file || !acct_globals.needcheck)
		goto out;
	spin_unlock(&acct_globals.lock);

	/* May block */
	if (vfs_statfs(file->f_path.dentry, &sbuf))
		return res;
	suspend = sbuf.f_blocks * SUSPEND;
	resume = sbuf.f_blocks * RESUME;

	sector_div(suspend, 100);
	sector_div(resume, 100);

	if (sbuf.f_bavail <= suspend)
		act = -1;
	else if (sbuf.f_bavail >= resume)
		act = 1;
	else
		act = 0;

	/*
	 * If some joker switched acct_globals.file under us we'ld better be
	 * silent and _not_ touch anything.
	 */
	spin_lock(&acct_globals.lock);
	if (file != acct_globals.file) {
		if (act)
			res = act>0;
		goto out;
	}

	if (acct_globals.active) {
		if (act < 0) {
			acct_globals.active = 0;
			printk(KERN_INFO "Process accounting paused\n");
		}
	} else {
		if (act > 0) {
			acct_globals.active = 1;
			printk(KERN_INFO "Process accounting resumed\n");
		}
	}

	del_timer(&acct_globals.timer);
	acct_globals.needcheck = 0;
	acct_globals.timer.expires = jiffies + ACCT_TIMEOUT*HZ;
	add_timer(&acct_globals.timer);
	res = acct_globals.active;
out:
	spin_unlock(&acct_globals.lock);
	return res;
}

/*
 * Close the old accounting file (if currently open) and then replace
 * it with file (if non-NULL).
 *
 * NOTE: acct_globals.lock MUST be held on entry and exit.
 */
static void acct_file_reopen(struct file *file)
{
	struct file *old_acct = NULL;

	if (acct_globals.file) {
		old_acct = acct_globals.file;
		del_timer(&acct_globals.timer);
		acct_globals.active = 0;
		acct_globals.needcheck = 0;
		acct_globals.file = NULL;
	}
	if (file) {
		acct_globals.file = file;
		acct_globals.needcheck = 0;
		acct_globals.active = 1;
		/* It's been deleted if it was used before so this is safe */
		init_timer(&acct_globals.timer);
		acct_globals.timer.function = acct_timeout;
		acct_globals.timer.expires = jiffies + ACCT_TIMEOUT*HZ;
		add_timer(&acct_globals.timer);
	}
	if (old_acct) {
		mnt_unpin(old_acct->f_path.mnt);
		spin_unlock(&acct_globals.lock);
		do_acct_process(old_acct);
		filp_close(old_acct, NULL);
		spin_lock(&acct_globals.lock);
	}
}

static int acct_on(char *name)
{
	struct file *file;
	int error;

	/* Difference from BSD - they don't do O_APPEND */
	file = filp_open(name, O_WRONLY|O_APPEND|O_LARGEFILE, 0);
	if (IS_ERR(file))
		return PTR_ERR(file);

	if (!S_ISREG(file->f_path.dentry->d_inode->i_mode)) {
		filp_close(file, NULL);
		return -EACCES;
	}

	if (!file->f_op->write) {
		filp_close(file, NULL);
		return -EIO;
	}

	error = security_acct(file);
	if (error) {
		filp_close(file, NULL);
		return error;
	}

	spin_lock(&acct_globals.lock);
	mnt_pin(file->f_path.mnt);
	acct_file_reopen(file);
	spin_unlock(&acct_globals.lock);

	mntput(file->f_path.mnt); /* it's pinned, now give up active reference */

	return 0;
}

/**
 * sys_acct - enable/disable process accounting
 * @name: file name for accounting records or NULL to shutdown accounting
 *
 * Returns 0 for success or negative errno values for failure.
 *
 * sys_acct() is the only system call needed to implement process
 * accounting. It takes the name of the file where accounting records
 * should be written. If the filename is NULL, accounting will be
 * shutdown.
 */
asmlinkage long sys_acct(const char __user *name)
{
	int error;

	if (!capable(CAP_SYS_PACCT))
		return -EPERM;

	if (name) {
		char *tmp = getname(name);
		if (IS_ERR(tmp))
			return (PTR_ERR(tmp));
		error = acct_on(tmp);
		putname(tmp);
	} else {
		error = security_acct(NULL);
		if (!error) {
			spin_lock(&acct_globals.lock);
			acct_file_reopen(NULL);
			spin_unlock(&acct_globals.lock);
		}
	}
	return error;
}

/**
 * acct_auto_close - turn off a filesystem's accounting if it is on
 * @m: vfsmount being shut down
 *
 * If the accounting is turned on for a file in the subtree pointed to
 * to by m, turn accounting off.  Done when m is about to die.
 */
void acct_auto_close_mnt(struct vfsmount *m)
{
	spin_lock(&acct_globals.lock);
	if (acct_globals.file && acct_globals.file->f_path.mnt == m)
		acct_file_reopen(NULL);
	spin_unlock(&acct_globals.lock);
}

/**
 * acct_auto_close - turn off a filesystem's accounting if it is on
 * @sb: super block for the filesystem
 *
 * If the accounting is turned on for a file in the filesystem pointed
 * to by sb, turn accounting off.
 */
void acct_auto_close(struct super_block *sb)
{
	spin_lock(&acct_globals.lock);
	if (acct_globals.file &&
	    acct_globals.file->f_path.mnt->mnt_sb == sb) {
		acct_file_reopen(NULL);
	}
	spin_unlock(&acct_globals.lock);
}

/*
 *  encode an unsigned long into a comp_t
 *
 *  This routine has been adopted from the encode_comp_t() function in
 *  the kern_acct.c file of the FreeBSD operating system. The encoding
 *  is a 13-bit fraction with a 3-bit (base 8) exponent.
 */

#define	MANTSIZE	13			/* 13 bit mantissa. */
#define	EXPSIZE		3			/* Base 8 (3 bit) exponent. */
#define	MAXFRACT	((1 << MANTSIZE) - 1)	/* Maximum fractional value. */

static comp_t encode_comp_t(unsigned long value)
{
	int exp, rnd;

	exp = rnd = 0;
	while (value > MAXFRACT) {
		rnd = value & (1 << (EXPSIZE - 1));	/* Round up? */
		value >>= EXPSIZE;	/* Base 8 exponent == 3 bit shift. */
		exp++;
	}

	/*
         * If we need to round up, do it (and handle overflow correctly).
         */
	if (rnd && (++value > MAXFRACT)) {
		value >>= EXPSIZE;
		exp++;
	}

	/*
         * Clean it up and polish it off.
         */
	exp <<= MANTSIZE;		/* Shift the exponent into place */
	exp += value;			/* and add on the mantissa. */
	return exp;
}

#if ACCT_VERSION==1 || ACCT_VERSION==2
/*
 * encode an u64 into a comp2_t (24 bits)
 *
 * Format: 5 bit base 2 exponent, 20 bits mantissa.
 * The leading bit of the mantissa is not stored, but implied for
 * non-zero exponents.
 * Largest encodable value is 50 bits.
 */

#define MANTSIZE2       20                      /* 20 bit mantissa. */
#define EXPSIZE2        5                       /* 5 bit base 2 exponent. */
#define MAXFRACT2       ((1ul << MANTSIZE2) - 1) /* Maximum fractional value. */
#define MAXEXP2         ((1 <<EXPSIZE2) - 1)    /* Maximum exponent. */

static comp2_t encode_comp2_t(u64 value)
{
        int exp, rnd;

        exp = (value > (MAXFRACT2>>1));
        rnd = 0;
        while (value > MAXFRACT2) {
                rnd = value & 1;
                value >>= 1;
                exp++;
        }

        /*
         * If we need to round up, do it (and handle overflow correctly).
         */
        if (rnd && (++value > MAXFRACT2)) {
                value >>= 1;
                exp++;
        }

        if (exp > MAXEXP2) {
                /* Overflow. Return largest representable number instead. */
                return (1ul << (MANTSIZE2+EXPSIZE2-1)) - 1;
        } else {
                return (value & (MAXFRACT2>>1)) | (exp << (MANTSIZE2-1));
        }
}
#endif

#if ACCT_VERSION==3
/*
 * encode an u64 into a 32 bit IEEE float
 */
static u32 encode_float(u64 value)
{
	unsigned exp = 190;
	unsigned u;

	if (value==0) return 0;
	while ((s64)value > 0){
		value <<= 1;
		exp--;
	}
	u = (u32)(value >> 40) & 0x7fffffu;
	return u | (exp << 23);
}
#endif

/*
 *  Write an accounting entry for an exiting process
 *
 *  The acct_process() call is the workhorse of the process
 *  accounting system. The struct acct is built here and then written
 *  into the accounting file. This function should only be called from
 *  do_exit().
 */

/*
 *  do_acct_process does all actual work. Caller holds the reference to file.
 */
static void do_acct_process(struct file *file)
{
	struct pacct_struct *pacct = &current->signal->pacct;
	acct_t ac;
	mm_segment_t fs;
	unsigned long flim;
	u64 elapsed;
	u64 run_time;
	struct timespec uptime;
	struct tty_struct *tty;

	/*
	 * First check to see if there is enough free_space to continue
	 * the process accounting system.
	 */
	if (!check_free_space(file))
		return;

	/*
	 * Fill the accounting struct with the needed info as recorded
	 * by the different kernel functions.
	 */
	memset((caddr_t)&ac, 0, sizeof(acct_t));

	ac.ac_version = ACCT_VERSION | ACCT_BYTEORDER;
	strlcpy(ac.ac_comm, current->comm, sizeof(ac.ac_comm));

	/* calculate run_time in nsec*/
	do_posix_clock_monotonic_gettime(&uptime);
	run_time = (u64)uptime.tv_sec*NSEC_PER_SEC + uptime.tv_nsec;
	run_time -= (u64)current->group_leader->start_time.tv_sec * NSEC_PER_SEC
		       + current->group_leader->start_time.tv_nsec;
	/* convert nsec -> AHZ */
	elapsed = nsec_to_AHZ(run_time);
#if ACCT_VERSION==3
	ac.ac_etime = encode_float(elapsed);
#else
	ac.ac_etime = encode_comp_t(elapsed < (unsigned long) -1l ?
	                       (unsigned long) elapsed : (unsigned long) -1l);
#endif
#if ACCT_VERSION==1 || ACCT_VERSION==2
	{
		/* new enlarged etime field */
		comp2_t etime = encode_comp2_t(elapsed);
		ac.ac_etime_hi = etime >> 16;
		ac.ac_etime_lo = (u16) etime;
	}
#endif
	do_div(elapsed, AHZ);
	ac.ac_btime = xtime.tv_sec - elapsed;
	/* we really need to bite the bullet and change layout */
	ac.ac_uid = current->uid;
	ac.ac_gid = current->gid;
#if ACCT_VERSION==2
	ac.ac_ahz = AHZ;
#endif
#if ACCT_VERSION==1 || ACCT_VERSION==2
	/* backward-compatible 16 bit fields */
	ac.ac_uid16 = current->uid;
	ac.ac_gid16 = current->gid;
#endif
#if ACCT_VERSION==3
	ac.ac_pid = current->tgid;
	ac.ac_ppid = current->parent->tgid;
#endif

	spin_lock_irq(&current->sighand->siglock);
	tty = current->signal->tty;
	ac.ac_tty = tty ? old_encode_dev(tty_devnum(tty)) : 0;
	ac.ac_utime = encode_comp_t(jiffies_to_AHZ(cputime_to_jiffies(pacct->ac_utime)));
	ac.ac_stime = encode_comp_t(jiffies_to_AHZ(cputime_to_jiffies(pacct->ac_stime)));
	ac.ac_flag = pacct->ac_flag;
	ac.ac_mem = encode_comp_t(pacct->ac_mem);
	ac.ac_minflt = encode_comp_t(pacct->ac_minflt);
	ac.ac_majflt = encode_comp_t(pacct->ac_majflt);
	ac.ac_exitcode = pacct->ac_exitcode;
	spin_unlock_irq(&current->sighand->siglock);
	ac.ac_io = encode_comp_t(0 /* current->io_usage */);	/* %% */
	ac.ac_rw = encode_comp_t(ac.ac_io / 1024);
	ac.ac_swaps = encode_comp_t(0);

	/*
         * Kernel segment override to datasegment and write it
         * to the accounting file.
         */
	fs = get_fs();
	set_fs(KERNEL_DS);
	/*
 	 * Accounting records are not subject to resource limits.
 	 */
	flim = current->signal->rlim[RLIMIT_FSIZE].rlim_cur;
	current->signal->rlim[RLIMIT_FSIZE].rlim_cur = RLIM_INFINITY;
	file->f_op->write(file, (char *)&ac,
			       sizeof(acct_t), &file->f_pos);
	current->signal->rlim[RLIMIT_FSIZE].rlim_cur = flim;
	set_fs(fs);
}

/**
 * acct_init_pacct - initialize a new pacct_struct
 * @pacct: per-process accounting info struct to initialize
 */
void acct_init_pacct(struct pacct_struct *pacct)
{
	memset(pacct, 0, sizeof(struct pacct_struct));
	pacct->ac_utime = pacct->ac_stime = cputime_zero;
}

/**
 * acct_collect - collect accounting information into pacct_struct
 * @exitcode: task exit code
 * @group_dead: not 0, if this thread is the last one in the process.
 */
void acct_collect(long exitcode, int group_dead)
{
	struct pacct_struct *pacct = &current->signal->pacct;
	unsigned long vsize = 0;

	if (group_dead && current->mm) {
		struct vm_area_struct *vma;
		down_read(&current->mm->mmap_sem);
		vma = current->mm->mmap;
		while (vma) {
			vsize += vma->vm_end - vma->vm_start;
			vma = vma->vm_next;
		}
		up_read(&current->mm->mmap_sem);
	}

	spin_lock_irq(&current->sighand->siglock);
	if (group_dead)
		pacct->ac_mem = vsize / 1024;
	if (thread_group_leader(current)) {
		pacct->ac_exitcode = exitcode;
		if (current->flags & PF_FORKNOEXEC)
			pacct->ac_flag |= AFORK;
	}
	if (current->flags & PF_SUPERPRIV)
		pacct->ac_flag |= ASU;
	if (current->flags & PF_DUMPCORE)
		pacct->ac_flag |= ACORE;
	if (current->flags & PF_SIGNALED)
		pacct->ac_flag |= AXSIG;
	pacct->ac_utime = cputime_add(pacct->ac_utime, current->utime);
	pacct->ac_stime = cputime_add(pacct->ac_stime, current->stime);
	pacct->ac_minflt += current->min_flt;
	pacct->ac_majflt += current->maj_flt;
	spin_unlock_irq(&current->sighand->siglock);
}

/**
 * acct_process - now just a wrapper around do_acct_process
 * @exitcode: task exit code
 *
 * handles process accounting for an exiting task
 */
void acct_process(void)
{
	struct file *file = NULL;

	/*
	 * accelerate the common fastpath:
	 */
	if (!acct_globals.file)
		return;

	spin_lock(&acct_globals.lock);
	file = acct_globals.file;
	if (unlikely(!file)) {
		spin_unlock(&acct_globals.lock);
		return;
	}
	get_file(file);
	spin_unlock(&acct_globals.lock);

	do_acct_process(file);
	fput(file);
}
/* audit.c -- Auditing support
 * Gateway between the kernel (e.g., selinux) and the user-space audit daemon.
 * System-call specific features have moved to auditsc.c
 *
 * Copyright 2003-2007 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Written by Rickard E. (Rik) Faith <faith@redhat.com>
 *
 * Goals: 1) Integrate fully with SELinux.
 *	  2) Minimal run-time overhead:
 *	     a) Minimal when syscall auditing is disabled (audit_enable=0).
 *	     b) Small when syscall auditing is enabled and no audit record
 *		is generated (defer as much work as possible to record
 *		generation time):
 *		i) context is allocated,
 *		ii) names from getname are stored without a copy, and
 *		iii) inode information stored from path_lookup.
 *	  3) Ability to disable syscall auditing at boot time (audit=0).
 *	  4) Usable by other parts of the kernel (if audit_log* is called,
 *	     then a syscall record will be generated automatically for the
 *	     current syscall).
 *	  5) Netlink interface to user-space.
 *	  6) Support low-overhead kernel-based filtering to minimize the
 *	     information that must be passed to user-space.
 *
 * Example user-space utilities: http://people.redhat.com/sgrubb/audit/
 */

#include <linux/init.h>
#include <asm/types.h>
#include <asm/atomic.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/kthread.h>

#include <linux/audit.h>

#include <net/sock.h>
#include <net/netlink.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/selinux.h>
#include <linux/inotify.h>
#include <linux/freezer.h>

#include "audit.h"

/* No auditing will take place until audit_initialized != 0.
 * (Initialization happens after skb_init is called.) */
static int	audit_initialized;

/* 0 - no auditing
 * 1 - auditing enabled
 * 2 - auditing enabled and configuration is locked/unchangeable. */
int		audit_enabled;

/* Default state when kernel boots without any parameters. */
static int	audit_default;

/* If auditing cannot proceed, audit_failure selects what happens. */
static int	audit_failure = AUDIT_FAIL_PRINTK;

/* If audit records are to be written to the netlink socket, audit_pid
 * contains the (non-zero) pid. */
int		audit_pid;

/* If audit_rate_limit is non-zero, limit the rate of sending audit records
 * to that number per second.  This prevents DoS attacks, but results in
 * audit records being dropped. */
static int	audit_rate_limit;

/* Number of outstanding audit_buffers allowed. */
static int	audit_backlog_limit = 64;
static int	audit_backlog_wait_time = 60 * HZ;
static int	audit_backlog_wait_overflow = 0;

/* The identity of the user shutting down the audit system. */
uid_t		audit_sig_uid = -1;
pid_t		audit_sig_pid = -1;
u32		audit_sig_sid = 0;

/* Records can be lost in several ways:
   0) [suppressed in audit_alloc]
   1) out of memory in audit_log_start [kmalloc of struct audit_buffer]
   2) out of memory in audit_log_move [alloc_skb]
   3) suppressed due to audit_rate_limit
   4) suppressed due to audit_backlog_limit
*/
static atomic_t    audit_lost = ATOMIC_INIT(0);

/* The netlink socket. */
static struct sock *audit_sock;

/* Inotify handle. */
struct inotify_handle *audit_ih;

/* Hash for inode-based rules */
struct list_head audit_inode_hash[AUDIT_INODE_BUCKETS];

/* The audit_freelist is a list of pre-allocated audit buffers (if more
 * than AUDIT_MAXFREE are in use, the audit buffer is freed instead of
 * being placed on the freelist). */
static DEFINE_SPINLOCK(audit_freelist_lock);
static int	   audit_freelist_count;
static LIST_HEAD(audit_freelist);

static struct sk_buff_head audit_skb_queue;
static struct task_struct *kauditd_task;
static DECLARE_WAIT_QUEUE_HEAD(kauditd_wait);
static DECLARE_WAIT_QUEUE_HEAD(audit_backlog_wait);

/* Serialize requests from userspace. */
static DEFINE_MUTEX(audit_cmd_mutex);

/* AUDIT_BUFSIZ is the size of the temporary buffer used for formatting
 * audit records.  Since printk uses a 1024 byte buffer, this buffer
 * should be at least that large. */
#define AUDIT_BUFSIZ 1024

/* AUDIT_MAXFREE is the number of empty audit_buffers we keep on the
 * audit_freelist.  Doing so eliminates many kmalloc/kfree calls. */
#define AUDIT_MAXFREE  (2*NR_CPUS)

/* The audit_buffer is used when formatting an audit record.  The caller
 * locks briefly to get the record off the freelist or to allocate the
 * buffer, and locks briefly to send the buffer to the netlink layer or
 * to place it on a transmit queue.  Multiple audit_buffers can be in
 * use simultaneously. */
struct audit_buffer {
	struct list_head     list;
	struct sk_buff       *skb;	/* formatted skb ready to send */
	struct audit_context *ctx;	/* NULL or associated context */
	gfp_t		     gfp_mask;
};

static void audit_set_pid(struct audit_buffer *ab, pid_t pid)
{
	struct nlmsghdr *nlh = nlmsg_hdr(ab->skb);
	nlh->nlmsg_pid = pid;
}

void audit_panic(const char *message)
{
	switch (audit_failure)
	{
	case AUDIT_FAIL_SILENT:
		break;
	case AUDIT_FAIL_PRINTK:
		printk(KERN_ERR "audit: %s\n", message);
		break;
	case AUDIT_FAIL_PANIC:
		panic("audit: %s\n", message);
		break;
	}
}

static inline int audit_rate_check(void)
{
	static unsigned long	last_check = 0;
	static int		messages   = 0;
	static DEFINE_SPINLOCK(lock);
	unsigned long		flags;
	unsigned long		now;
	unsigned long		elapsed;
	int			retval	   = 0;

	if (!audit_rate_limit) return 1;

	spin_lock_irqsave(&lock, flags);
	if (++messages < audit_rate_limit) {
		retval = 1;
	} else {
		now     = jiffies;
		elapsed = now - last_check;
		if (elapsed > HZ) {
			last_check = now;
			messages   = 0;
			retval     = 1;
		}
	}
	spin_unlock_irqrestore(&lock, flags);

	return retval;
}

/**
 * audit_log_lost - conditionally log lost audit message event
 * @message: the message stating reason for lost audit message
 *
 * Emit at least 1 message per second, even if audit_rate_check is
 * throttling.
 * Always increment the lost messages counter.
*/
void audit_log_lost(const char *message)
{
	static unsigned long	last_msg = 0;
	static DEFINE_SPINLOCK(lock);
	unsigned long		flags;
	unsigned long		now;
	int			print;

	atomic_inc(&audit_lost);

	print = (audit_failure == AUDIT_FAIL_PANIC || !audit_rate_limit);

	if (!print) {
		spin_lock_irqsave(&lock, flags);
		now = jiffies;
		if (now - last_msg > HZ) {
			print = 1;
			last_msg = now;
		}
		spin_unlock_irqrestore(&lock, flags);
	}

	if (print) {
		printk(KERN_WARNING
		       "audit: audit_lost=%d audit_rate_limit=%d audit_backlog_limit=%d\n",
		       atomic_read(&audit_lost),
		       audit_rate_limit,
		       audit_backlog_limit);
		audit_panic(message);
	}
}

static int audit_set_rate_limit(int limit, uid_t loginuid, u32 sid)
{
	int res, rc = 0, old = audit_rate_limit;

	/* check if we are locked */
	if (audit_enabled == 2)
		res = 0;
	else
		res = 1;

	if (sid) {
		char *ctx = NULL;
		u32 len;
		if ((rc = selinux_sid_to_string(sid, &ctx, &len)) == 0) {
			audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
				"audit_rate_limit=%d old=%d by auid=%u"
				" subj=%s res=%d",
				limit, old, loginuid, ctx, res);
			kfree(ctx);
		} else
			res = 0; /* Something weird, deny request */
	}
	audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
		"audit_rate_limit=%d old=%d by auid=%u res=%d",
		limit, old, loginuid, res);

	/* If we are allowed, make the change */
	if (res == 1)
		audit_rate_limit = limit;
	/* Not allowed, update reason */
	else if (rc == 0)
		rc = -EPERM;
	return rc;
}

static int audit_set_backlog_limit(int limit, uid_t loginuid, u32 sid)
{
	int res, rc = 0, old = audit_backlog_limit;

	/* check if we are locked */
	if (audit_enabled == 2)
		res = 0;
	else
		res = 1;

	if (sid) {
		char *ctx = NULL;
		u32 len;
		if ((rc = selinux_sid_to_string(sid, &ctx, &len)) == 0) {
			audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
				"audit_backlog_limit=%d old=%d by auid=%u"
				" subj=%s res=%d",
				limit, old, loginuid, ctx, res);
			kfree(ctx);
		} else
			res = 0; /* Something weird, deny request */
	}
	audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
		"audit_backlog_limit=%d old=%d by auid=%u res=%d",
		limit, old, loginuid, res);

	/* If we are allowed, make the change */
	if (res == 1)
		audit_backlog_limit = limit;
	/* Not allowed, update reason */
	else if (rc == 0)
		rc = -EPERM;
	return rc;
}

static int audit_set_enabled(int state, uid_t loginuid, u32 sid)
{
	int res, rc = 0, old = audit_enabled;

	if (state < 0 || state > 2)
		return -EINVAL;

	/* check if we are locked */
	if (audit_enabled == 2)
		res = 0;
	else
		res = 1;

	if (sid) {
		char *ctx = NULL;
		u32 len;
		if ((rc = selinux_sid_to_string(sid, &ctx, &len)) == 0) {
			audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
				"audit_enabled=%d old=%d by auid=%u"
				" subj=%s res=%d",
				state, old, loginuid, ctx, res);
			kfree(ctx);
		} else
			res = 0; /* Something weird, deny request */
	}
	audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
		"audit_enabled=%d old=%d by auid=%u res=%d",
		state, old, loginuid, res);

	/* If we are allowed, make the change */
	if (res == 1)
		audit_enabled = state;
	/* Not allowed, update reason */
	else if (rc == 0)
		rc = -EPERM;
	return rc;
}

static int audit_set_failure(int state, uid_t loginuid, u32 sid)
{
	int res, rc = 0, old = audit_failure;

	if (state != AUDIT_FAIL_SILENT
	    && state != AUDIT_FAIL_PRINTK
	    && state != AUDIT_FAIL_PANIC)
		return -EINVAL;

	/* check if we are locked */
	if (audit_enabled == 2)
		res = 0;
	else
		res = 1;

	if (sid) {
		char *ctx = NULL;
		u32 len;
		if ((rc = selinux_sid_to_string(sid, &ctx, &len)) == 0) {
			audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
				"audit_failure=%d old=%d by auid=%u"
				" subj=%s res=%d",
				state, old, loginuid, ctx, res);
			kfree(ctx);
		} else
			res = 0; /* Something weird, deny request */
	}
	audit_log(NULL, GFP_KERNEL, AUDIT_CONFIG_CHANGE,
		"audit_failure=%d old=%d by auid=%u res=%d",
		state, old, loginuid, res);

	/* If we are allowed, make the change */
	if (res == 1)
		audit_failure = state;
	/* Not allowed, update reason */
	else if (rc == 0)
		rc = -EPERM;
	return rc;
}

continue
continue
continue
"End of Example"

if &cp | set nocp | endif
let s:cpo_save=&cpo
set cpo&vim
inoremap <silent> <S-Tab> =BackwardsSnippet()
inoremap <C-Tab> 	
snoremap <silent> 	 i<Right>=TriggerSnippet()
nmap <silent>  :BottomExplorerWindow
nmap <silent>  :FirstExplorerWindow
snoremap  b<BS>
nmap <silent>  k :wincmd k
nmap <silent>  h :wincmd h
nmap <silent>  j :wincmd j
nmap <silent>    :CommandT
nmap <silent>  r :CommandTFlush
nmap <silent>  q :FufQuickfix
nmap <silent>  t :FufTag
nmap <silent>  l :wincmd l
nmap <silent>  m :FufMruFile
nmap <silent>  f :FufFile
nmap <silent>  d :FufDir
nmap <silent>  b :CommandTBuffer
snoremap % b<BS>%
snoremap ' b<BS>'
nmap <silent> 2<F8> :make -j2
nmap <silent> 3<F8> :make -j3
nmap <silent> 4<F8> :make -j4
map Q gq
xmap S <Plug>VSurround
snoremap U b<BS>U
vmap [% [%m'gv``
snoremap \ b<BS>\
nmap <silent> \gh :GitStash
nmap <silent> \gs :GitStatus
nmap <silent> \gl :GitPull
nmap <silent> \gp :GitPush
nmap <silent> \G :ToggleGitMenu
nmap <silent> \ga :GitAdd
nmap <silent> \ca :GitCommitAll
nmap <silent> \ci :GitCommit
map \dk <Plug>DirDiffPrev
map \dj <Plug>DirDiffNext
map \dp <Plug>DirDiffPut
map \dg <Plug>DirDiffGet
nmap <silent> \ig :IndentGuidesToggle
nmap <silent> \vf :Vst fold
nmap <silent> \r :e
nmap <silent> \w :set invwrap
nmap <silent> \ds :%s/\s\+$//g
nmap <silent> \vl :source $MYVIMRC
nmap <silent> \ve :e $MYVIMRC
nmap <silent> \t :!ctags -R .
nmap <silent> \st :GitStatus
nmap <silent> \ntm :NERDTreeMirror
nmap <silent> \ntf :NERDTreeFind
nmap <silent> \nt :NERDTreeToggle
vmap ]% ]%m'gv``
snoremap ^ b<BS>^
snoremap ` b<BS>`
vmap a% [%v]%
nmap cs <Plug>Csurround
nmap ds <Plug>Dsurround
nmap gx <Plug>NetrwBrowseX
xmap gS <Plug>VgSurround
xmap s <Plug>Vsurround
nmap ySS <Plug>YSsurround
nmap ySs <Plug>YSsurround
nmap yss <Plug>Yssurround
nmap yS <Plug>YSurround
nmap ys <Plug>Ysurround
snoremap <Left> bi
snoremap <Right> a
snoremap <BS> b<BS>
snoremap <silent> <S-Tab> i<Right>=BackwardsSnippet()
nnoremap <silent> <Plug>NetrwBrowseX :call netrw#NetrwBrowseX(expand("<cWORD>"),0)
nmap <silent> <F10> :set invpaste
nmap <silent> <F9> :copen
nmap <silent> <F8> :make
nmap <silent> <F4> :set invhlsearch
nmap <silent> <F3> :close
nmap <silent> <F2> :tabnew
nmap <silent> <F11> :call ReloadAllSnippets()
nmap <silent> <F7> :WMToggle
nmap <silent> <F5> :BufExplorer
nmap <silent> <F6> :TlistUpdate
inoremap <silent> o =VST_Ornaments()
imap S <Plug>ISurround
imap s <Plug>Isurround
inoremap <silent> 	 =TriggerSnippet()
imap  <Plug>SuperTabForward
imap  <Plug>SuperTabBackward
inoremap <silent> 	 =ShowAvailableSnips()
imap  <Plug>Isurround
inoremap  u
cabbr gmrename GitRemoteRename
cabbr gmdel GitRemoteDel
cabbr gmadd GitRemoteAdd
cabbr gpull GitPull
cabbr gpush GitPush
cabbr glog GitLog
let &cpo=s:cpo_save
unlet s:cpo_save
set autoindent
set background=dark
set backspace=indent,eol,start
set expandtab
set fileencodings=utf-8,gbk,ucs-bom,ucs-2,latin1
set grepprg=grep\ -nH\ $*
set helplang=cn
set hlsearch
set ignorecase
set incsearch
set laststatus=2
set makeprg=scons
set mouse=a
set path=.,/usr/include,,,/usr/local/include,**/
set printoptions=paper:letter
set ruler
set runtimepath=~/.vim,~/.vim/bundle/bufexplorer,~/.vim/bundle/coffee-script,~/.vim/bundle/command-t,~/.vim/bundle/dirdiff,~/.vim/bundle/fuzzyfinder,~/.vim/bundle/hypergit,~/.vim/bundle/l9,~/.vim/bundle/markdown,~/.vim/bundle/matchit,~/.vim/bundle/nerdtree,~/.vim/bundle/snipmate,~/.vim/bundle/supertab,~/.vim/bundle/surround,~/.vim/bundle/taglist-plus,~/.vim/bundle/vimgdb,~/.vim/bundle/vst-with-syn,~/.vim/bundle/winmanager,~/.vim/bundle/yaifa,/var/lib/vim/addons,/usr/share/vim/vimfiles,/usr/share/vim/vim73,/usr/share/vim/vimfiles/after,/var/lib/vim/addons/after,~/.vim/bundle/coffee-script/after,~/.vim/bundle/hypergit/after,~/.vim/bundle/snipmate/after,~/.vim/after
set shiftwidth=4
set showmatch
set softtabstop=4
set suffixes=.bak,~,.swp,.o,.info,.aux,.log,.dvi,.bbl,.blg,.brf,.cb,.ind,.idx,.ilg,.inx,.out,.toc
set noswapfile
set termencoding=utf-8
set updatetime=1
set virtualedit=block
set whichwrap=b,s,h,l,<,>,~,[,]
set wildignore=*.pyc
set wildmenu
" vim: set ft=vim :

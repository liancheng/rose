(set-current-output-port! (current-error-port))

((lambda (yin)
   ((lambda (yang)
      (yin yang))
    ((lambda (cc) (display #\*) cc) (call/cc (lambda (c) c)))))
 ((lambda (cc) (display #\@) cc) (call/cc (lambda (c) c))))

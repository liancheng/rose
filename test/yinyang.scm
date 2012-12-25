(define yin ((lambda (k) k) (call/cc (lambda (c) c))))
(define yang ((lambda (k) k) (call/cc (lambda (c) c))))
(yin yang)

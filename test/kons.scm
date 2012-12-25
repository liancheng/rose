(define kons
  (lambda (head tail)
    (lambda (k)
      (if k head tail))))

(define kar
  (lambda (pair)
    (pair #t)))

(define kdr
  (lambda (pair)
    (pair #f)))

(define x (kons 1 2))

(kdr x)

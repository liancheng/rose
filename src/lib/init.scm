;;; Pair procedures

(define (caar pair) (car (car pair)))
(define (cadr pair) (car (cdr pair)))
(define (cdar pair) (cdr (car pair)))
(define (cddr pair) (cdr (cdr pair)))

(define (caaar pair) (car (caar pair)))
(define (caadr pair) (car (cadr pair)))
(define (cadar pair) (car (cdar pair)))
(define (caddr pair) (car (cddr pair)))
(define (cdaar pair) (cdr (caar pair)))
(define (cdadr pair) (cdr (cadr pair)))
(define (cddar pair) (cdr (cdar pair)))
(define (cdddr pair) (cdr (cddr pair)))

(define (caaaar pair) (car (caaar pair)))
(define (caaadr pair) (car (caadr pair)))
(define (caadar pair) (car (cadar pair)))
(define (caaddr pair) (car (caddr pair)))
(define (cadaar pair) (car (cdaar pair)))
(define (cadadr pair) (car (cdadr pair)))
(define (caddar pair) (car (cddar pair)))
(define (cadddr pair) (car (cdddr pair)))
(define (cdaaar pair) (cdr (caaar pair)))
(define (cdaadr pair) (cdr (caadr pair)))
(define (cdadar pair) (cdr (cadar pair)))
(define (cdaddr pair) (cdr (caddr pair)))
(define (cddaar pair) (cdr (cdaar pair)))
(define (cddadr pair) (cdr (cdadr pair)))
(define (cdddar pair) (cdr (cddar pair)))
(define (cddddr pair) (cdr (cdddr pair)))

(define (list? obj)
  (if (null? obj)
    #t
    (if (pair? obj)
      (list? (cdr obj))
      #f)))

(define (length list)
  (if (null? list)
    0
    (+ 1 (length (cdr list)))))

; (define (append list . args)
;   (define (append* list args)
;     (if (null? list)
;       args
;       (cons (car list)
;             (append* (cdr list) args))))
;   (append* list args))

(define (list-tail list k)
  (if (zero? k)
    list
    (list-tail (cdr list) (- k 1))))

(define (list-ref list k)
  (if (zero? k)
    (car list)
    (list-ref (cdr list) (- k 1))))

(define (memq obj list)
  (if (null? list)
    #f
    (if (eq? obj (car list))
      list
      (memq obj (cdr list)))))

(define (memv obj list)
  (if (null? list)
    #f
    (if (eqv? obj (car list))
      list
      (memv obj (cdr list)))))

(define (member obj list)
  (if (null? list)
    #f
    (if (equal? obj (car list))
      list
      (memv obj (cdr list)))))

(define (assq obj alist)
  (if (null? alist)
    #f
    (if (eq? obj (caar alist))
      (car alist)
      (assq obj (cdr alist)))))

(define (assv obj alist)
  (if (null? alist)
    #f
    (if (eqv? obj (caar alist))
      (car alist)
      (assq obj (cdr alist)))))

(define (assoc obj alist)
  (if (null? alist)
    #f
    (if (equal? obj (caar alist))
      (car alist)
      (assq obj (cdr alist)))))

;;; Number procedures

(define (zero? n)
  (= 0 n))

(define (positive? n)
  (> n 0))

(define (negative? n)
  (< n 0))

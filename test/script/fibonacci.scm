(define (fibonacci n)
  (define (fibonacci* n k k-1 k-2)
    (if (= n k)
      (+ k-1 k-2)
      (fibonacci* n (+ k 1) (+ k-1 k-2) k-1)))
  (if (= n 0)
    0
    (if (= n 1)
      1
      (fibonacci* n 2 1 0))))

(define (slow-fibonacci n)
  (if (= n 0)
    0
    (if (= n 1)
      1
      (+ (slow-fibonacci (- n 1))
         (slow-fibonacci (- n 2))))))

(slow-fibonacci 20)

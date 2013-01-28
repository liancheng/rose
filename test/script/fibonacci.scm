(define (slow-fibonacci m)
  (if (= m 0)
    0
    (if (= m 1)
      1
      (+ (slow-fibonacci (- m 1))
         (slow-fibonacci (- m 2))))))

(display (slow-fibonacci (read)))
(newline)

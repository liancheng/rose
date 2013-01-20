(define (factorial n)
  (display n)
  (newline)
  (if (= n 0)
    1
    (* n (factorial (- n 1)))))

(display (factorial (read)))
(newline)

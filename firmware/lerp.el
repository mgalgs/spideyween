(defun lerp (x0 x1 y0 y1 x)
  "Interpolate x from the range [x0..x1] onto [y0..y1].  All
parameters and return value are integers."
  (setq x0 (float x0)
        x1 (float x1)
        y0 (float y0)
        y1 (float y1))
  (floor (+ y0 (* (- y1 y0) (/ (- x x0) (- x1 x0))))))

(defun calculate-lookup-table ()
  (let ((i 0)
        table)
    (while (< i 180)
      (push (lerp 0 180 2000 4000 i) table)
      (setq i (1+ i)))
    (reverse table)))

(defun insert-lookup-table ()
  (insert (mapconcat 'number-to-string (calculate-lookup-table) ", ")))

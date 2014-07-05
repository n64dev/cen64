.code
fpu_cvt_f64_i64 proc
  fclex
  fild QWORD PTR [rcx]
  fstp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f64_i64 endp
end


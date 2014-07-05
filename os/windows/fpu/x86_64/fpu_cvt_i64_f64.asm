.code
fpu_cvt_i64_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fistp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_i64_f64 endp
end


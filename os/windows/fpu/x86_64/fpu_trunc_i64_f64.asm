.code
fpu_trunc_i64_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fisttp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_trunc_i64_f64 endp
end


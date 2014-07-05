.code
fpu_trunc_i32_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fisttp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_trunc_i32_f64 endp
end


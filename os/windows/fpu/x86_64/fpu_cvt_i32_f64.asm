.code
fpu_cvt_i32_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fistp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_i32_f64 endp
end


.code
fpu_cvt_f32_f64 proc
  fclex
  fld QWORD PTR [rcx]
  fstp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f32_f64 endp
end


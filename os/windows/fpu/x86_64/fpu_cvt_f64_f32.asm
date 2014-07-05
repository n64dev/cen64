.code
fpu_cvt_f64_f32 proc
  fclex
  fld DWORD PTR [rcx]
  fstp QWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f64_f32 endp
end


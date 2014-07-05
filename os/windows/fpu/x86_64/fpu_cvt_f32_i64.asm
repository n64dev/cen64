.code
fpu_cvt_f32_i64 proc
  fclex
  fild QWORD PTR [rcx]
  fstp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f32_i64 endp
end


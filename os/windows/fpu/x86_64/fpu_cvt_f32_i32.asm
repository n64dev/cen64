.code
fpu_cvt_f32_i32 proc
  fclex
  fild DWORD PTR [rcx]
  fstp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_f32_i32 endp
end


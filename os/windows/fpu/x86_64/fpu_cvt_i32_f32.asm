.code
fpu_cvt_i32_f32 proc
  fclex
  fld DWORD PTR [rcx]
  fistp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_cvt_i32_f32 endp
end


.code
fpu_trunc_i32_f32 proc
  fclex
  fld DWORD PTR [rcx]
  fisttp DWORD PTR [rdx]
  fstsw ax
  ret
fpu_trunc_i32_f32 endp
end


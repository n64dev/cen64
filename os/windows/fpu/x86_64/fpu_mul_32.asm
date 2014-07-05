.code
fpu_mul_32 proc
  fclex
  fld DWORD PTR [rcx]
  fld DWORD PTR [rdx]
  fmulp
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_mul_32 endp
end


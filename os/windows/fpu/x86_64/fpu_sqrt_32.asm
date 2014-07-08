.code
fpu_sqrt_32 proc
  fclex
  fld DWORD PTR [rcx]
  fsqrt
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_sqrt_32 endp
end


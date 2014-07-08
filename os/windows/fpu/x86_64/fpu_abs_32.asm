.code
fpu_abs_32 proc
  fclex
  fld DWORD PTR [rcx]
  fabs
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_abs_32 endp
end


.code
fpu_add_32 proc
  fclex
  fld DWORD PTR [rcx]
  fld DWORD PTR [rdx]
  faddp
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_add_32 endp
end


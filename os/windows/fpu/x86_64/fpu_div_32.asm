.code
fpu_div_32 proc
  fclex
  fld DWORD PTR [rcx]
  fld DWORD PTR [rdx]
  fdivp
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_div_32 endp
end


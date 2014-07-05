.code
fpu_div_32 proc
  fclex
  fld DWORD PTR [rcx]
  fld DWORD PTR [rdx]
  fdivrp
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_div_32 endp
end


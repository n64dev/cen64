.code
fpu_neg_32 proc
  fclex
  fld DWORD PTR [rcx]
  fchs
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_neg_32 endp
end


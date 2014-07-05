.code
fpu_sub_32 proc
  fclex
  fld DWORD PTR [rcx]
  fld DWORD PTR [rdx]
  fsubrp
  fstp DWORD PTR [r8]
  fstsw ax
  ret
fpu_sub_32 endp
end


.code
fpu_add_64 proc
  fclex
  fld QWORD PTR [rcx]
  fld QWORD PTR [rdx]
  faddp
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_add_64 endp
end


.code
fpu_sub_64 proc
  fclex
  fld QWORD PTR [rcx]
  fld QWORD PTR [rdx]
  fsubrp
  fstp QWORD PTR [r8]
  fstsw ax
  ret
fpu_sub_64 endp
end


" Vim syntax file
" Language: Variational Feature Language
" Maintainer: Bradley Worley
" Latest Revision: 04 May 2017

if exists("b:current_syntax")
  finish
endif

" Keywords
syn keyword vflKeyword for in
syn keyword vflConstant true false nil
syn keyword vflTodo contained TODO FIXME XXX

" Matches
syn match vflComment "^#.*" contains=vflTodo
syn match vflComment "\s#.*"ms=s+1 contains=vflTodo
syn match vflNum "[-+]\?\(\d\+\|\d*\.\d\+\|\d\+\.\d*\)\([eE][-+]\?\d\+\)\?"
syn match vflType "[a-zA-Z0-9]\+:"

" Regions
syn region vflString start=+'+ skip=+\\\\\|\\'+ end=+'+ oneline

" Default highlighting
hi def link vflConstant Constant
hi def link vflComment  Comment
hi def link vflTodo     Todo
hi def link vflString   String
hi def link vflNum      Number
hi def link vflType     Type
hi def link vflKeyword  Statement

let b:current_syntax = "vflang"


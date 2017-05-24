" ZASM Syntax File
" Language:      ZASM
" Maintainer:    cmj <cmj@cmj.tw>
" Last Change:   2016 Dec 14
"

if exists("b:current_syntax")
    finish
endif

let s:cpo_save = &cpo
set cpo&vim

let zerg_asm_highlight=1

" Comment on ZASM
"
" All comment are starts with # in one line
" If need the multiple-line comment, using long string without assign
" to any identifier, ZASM will skip process this string and consider
" as the comments
"
syn match   ZASMComment       "#.*$" contains=ZASMToDo,@Spell
syn keyword ZASMToDo          contained XXX TODO FIXME NOTE HACK
syn keyword ZASMReserved      define
syn keyword ZASMInclude       include
syn match   ZASMNumber        "\<\d\+\>"
syn match   ZASMNumber        "\<0[oO]\=\o\+[Ll]\=\>"
syn match   ZASMNumber        "\<0[xX]\x\+[Ll]\=\>"
syn match   ZASMNumber        "\<0[bB][01]\+[Ll]\=\>"
syn match   ZASMNumber        "\<\%([1-9]\d*\|0\)[Ll]\=\>"
syn match   ZASMFloat         "\<\d\+\.\=\%([eE][+-]\=\d\+\)\="
syn match   ZASMOperator      "\w\+" contains=ZASMReserved,ZASMComment,ZASMNumber,ZASMString,ZASMTag
syn match   ZASMIdentifier    "&\w\+"
syn match   ZASMTag           contained "^\s*\w\+:"
syn region  ZASMString        start=+\z(['"]\)+ end="\z1" skip="\\\z1" contains=ZASMEscape
syn match   ZASMEscape        contained +\\[abfnrtv'"\\]+
syn match   ZASMEscape        contained +\\x\x\{2}+
syn keyword ZASMStructure     byte word dword qword


" ZASM highlight syntax definition
"
hi def link ZASMComment         Comment
hi def link ZASMToDo            Todo
hi def link ZASMReserved        Statement
hi def link ZASMNumber          Number
hi def link ZASMFloat           Number
hi def link ZASMOperator        Function
hi def link ZASMIdentifier      Type
hi def link ZASMTag             Label
hi def link ZASMString          String
hi def link ZASMEscape          Special
hi def link ZASMStructure       Structure
hi def link ZASMInclude         Include

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: set spell!:

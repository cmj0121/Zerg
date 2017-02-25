" ZERG IR Syntax File
" Language:      ZERG
" Maintainer:    cmj <cmj@cmj.tw>
" Last Change:   2016 Dec 14
"

if exists("b:current_syntax")
    finish
endif

let s:cpo_save = &cpo
set cpo&vim

let zerg_ir_highlight=1

syn match   ZGRComment       "#.*$" contains=ZGRToDo,@Spell
syn keyword ZGRToDo          contained XXX TODO FIXME NOTE HACK
syn match   ZGRNumber        "\<\d\+\>"
syn match   ZGRNumber        "\<0[oO]\=\o\+[Ll]\=\>"
syn match   ZGRNumber        "\<0[xX]\x\+[Ll]\=\>"
syn match   ZGRNumber        "\<0[bB][01]\+[Ll]\=\>"
syn match   ZGRNumber        "\<\%([1-9]\d*\|0\)[Ll]\=\>"
syn match   ZGRReference     "&[\.a-zA-Z0-9_]\+"
syn match   ZGROpcode        "^\w\+" contained
syn match   ZGROperand       "\w\+" contains=ZGROpcode,ZGRNumber
syn match   ZGRRegister      "\.[\.a-zA-Z0-9_]\+"
syn region  ZGRString        start=+\z(['"]\)+ end="\z1" skip="\\\z1" contains=ZGREscape
syn match   ZGREscape        +\\[abfnrtv'"\\]+ contained
syn match   ZGREscape        +\\x\x\{2}+ contained
syn region  ZGRMemory        start=+\[+ end=+\]+ contained
syn match   ZGRInlineASM     "->.*$" contains=ZGRNumber,ZGRReference,ZGRMemory,ZGRRegister

hi def link ZGRComment   Comment
hi def link ZGRToDo      Todo
hi def link ZGRNumber    Number
hi def link ZGRReference Type
hi def link ZGROpcode    Statement
hi def link ZGROperand   Function
hi def link ZGRRegister  Structure
hi def link ZGRString    String
hi def link ZGREscape    Special
hi def link ZGRMemory    StorageClass
hi def link ZGRInlineASM Special

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: set spell!:

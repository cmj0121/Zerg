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

syn match   ZERGIRComment       "#.*$" contains=ZERGIRToDo,@Spell
syn keyword ZERGIRToDo          contained XXX TODO FIXME NOTE HACK
syn match   ZERGIRNumber        "\<\d\+\>"
syn match   ZERGIRNumber        "\<0[oO]\=\o\+[Ll]\=\>"
syn match   ZERGIRNumber        "\<0[xX]\x\+[Ll]\=\>"
syn match   ZERGIRNumber        "\<0[bB][01]\+[Ll]\=\>"
syn match   ZERGIRNumber        "\<\%([1-9]\d*\|0\)[Ll]\=\>"
syn match   ZERGIRReference     "&\w\+"
syn match   ZERGIROpcode        "^\w\+" contained
syn match   ZERGIROperand       "\w\+" contains=ZERGIROpcode,ZERGIRNumber
syn match   ZERGIRRegister      "\%(\.\w\+\)\+"
syn region  ZERGIRString        start=+\z(['"]\)+ end="\z1" skip="\\\z1" contains=ZERGIREscape
syn match   ZERGIREscape        +\\[abfnrtv'"\\]+ contained
syn match   ZERGIREscape        +\\x\x\{2}+ contained
syn region  ZERGIRMemory        start=+\[+ end=+\]+ contained
syn match   ZERGIRInlineASM     "->.*$" contains=ZERGIRNumber,ZERGIRReference,ZERGIRMemory

hi def link ZERGIRComment   Comment
hi def link ZERGIRToDo      Todo
hi def link ZERGIRNumber    Number
hi def link ZERGIRReference Type
hi def link ZERGIROpcode    Statement
hi def link ZERGIROperand   Function
hi def link ZERGIRRegister  Structure
hi def link ZERGIRString    String
hi def link ZERGIREscape    Special
hi def link ZERGIRMemory    StorageClass
hi def link ZERGIRInlineASM Special

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: set spell!:

;
; processing-utils.scm
;
; Utilities for applying different processing steps to input sentences.
; These include getting a list of recently parsed sentences, and a 
; utility to send raw input text to the RelEx parse server, to get the 
; text parsed.
; 
; Copyright (c) 2009 Linas Vepstas <linasvepstas@gmail.com>
;
; -----------------------------------------------------------------------
; global vars:
; new-sent anchor points at the node to which all new sentences are connected
;
(define new-parsed-sent-anchor (AnchorNode "# New Parsed Sentence" (stv 1 1)))

; Return the list of SentenceNodes that are attached to the 
; freshly-parsed anchor.  This list will be non-empty if relex-parse
; has been recently run. This list can be emptied with the call
; delete-new-parsed-sent-links below.
;
(define (get-new-parsed-sentences)
	(cog-chase-link 'ListLink 'SentenceNode new-parsed-sent-anchor)
)

; delete-new-parsed-sent-links deletes the links that anchor sentences to 
; to new-parsed-sent anchor.
;
(define (delete-new-parsed-sent-links)
	(for-each (lambda (x) (cog-delete x)) (cog-incoming-set new-parsed-sent-anchor))
)

; -----------------------------------------------------------------------
; relex-parse -- send text to RelEx parser, load the resulting opencog atoms
;
; This routine takes plain-text input (in english), and sends it off 
; to a running instance of the RelEx parser, which should be listening 
; on port 4444. The parser will return a set of atoms, and these are
; then loaded into this opencog instance. After import, these are attached
; to the "new-parsed-sent-anchor" via a ListLink; the set of newly added
; sentences can be fetched with the "get-new-parsed-sentences" call.
;
(define (relex-parse plain-txt)

	; A little short routine that sends the plain-text to the
	; RelEx parser, and then loads the resulting parse into the
	; atomspace (using exec-scm-from-port to do the load)
	(define (do-sock-io sent-txt)
		(let ((s (socket PF_INET SOCK_STREAM 0)))
			(connect s AF_INET (inet-aton "127.0.0.1") 4444)
	
			(display sent-txt s)
			(display "\n" s) ; must send newline to flush socket
			(system (string-join (list "echo Info: send to parser: " sent-txt)))
			(exec-scm-from-port s)
			(shutdown s 2)
			(system (string-join (list "echo Info: close socket to parser" )))
		)
	)

	; Perform the actual processing
	(do-sock-io plain-txt)
)

; -----------------------------------------------------------------------
; attach-parses-to-anchor -- given sentences, attach the parses to anchor.
; 
; Given a list of sentences i.e. a list of SentenceNodes, go through them,
; locate the ParseNodes, and attach the parse nodes to the anchor.
;
; return value is undefined (no return value).
;
(define (attach-parses-to-anchor sent-list anchor)

	;; Attach all parses of a sentence to the anchor.
	(define (attach-parses sent)
		;; Get list of parses for the sentence.
		(define (get-parses sent)
			(cog-chase-link 'ParseLink 'ParseNode sent)
		)
		;; Attach all parses of the sentence to the anchor.
		;; This must have a true/confident TV so that the pattern
		;; matcher will find and use this link.
		(for-each (lambda (x) (ListLink anchor x (stv 1 1)))
			(get-parses sent)
		)
	)
	;; Attach all parses of all sentences to the anchor.
	(for-each attach-parses sent-list)
)

; -----------------------------------------------------------------------
; -----------------------------------------------------------------------

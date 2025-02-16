;;; Directory Local Variables            -*- no-byte-compile: t -*-
;;; For more information see (info "(emacs) Directory Variables")

((nil . ((compile-command . "guix shell --pure -Df guix.scm -- make -j4 debug")
		 (intr . nil)
		 (gdb-args . "./build/bin/main")
		 (init-cmd . ((lambda nil
						(find-file "src/prog/main.cpp")
						(v66v-project-compile)))))))

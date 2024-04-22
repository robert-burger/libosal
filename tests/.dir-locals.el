;; this requires format-all and astyle to be installed,
;; and will auto-format all C++ files according to the options
;; in astylerc on saving
((c++-mode . ((eval . (setq format-all-formatters '(("C++" clang-format))))
	      (mode . format-all)))
 (python-mode . ((eval . (setq format-all-formatters '(("Python" black))))
		 (mode . format-all))))


(use-modules (guix)
             (guix build-system gnu)
             (guix build-system cmake)
             (guix git-download)
             (gnu packages tex)
             (gnu packages pulseaudio)
             (gnu packages commencement)
             (gnu packages networking)
             (gnu packages tls)
             (gnu packages admin)
             (gnu packages sqlite)
             (gnu packages cpp)
             (gnu packages gtk)
             (gnu packages parallel)
             (gnu packages xml)
             (gnu packages glib)
             (gnu packages python)
             (gnu packages python-science)
             (gnu packages python-xyz)
             (gnu packages compression)
             (gnu packages autotools)
             (gnu packages check)
             (gnu packages fontutils)
             (gnu packages pkg-config)
             ((guix licenses)
              #:prefix license:))

(define vcs-file?
  ;; Return true if the given file is under version control.
  (or (git-predicate (current-source-directory))
      (const #t)))

(define-public readerwriterqueue
  (package
   (name "readerwriterqueue")
   (version "1.0.6-dev")
   (source
    (origin
     (method git-fetch)
     (uri (git-reference
           (url "https://github.com/cameron314/readerwriterqueue/")
           (commit "16b48ae1148284e7b40abf72167206a4390a4592")))
     (file-name (git-file-name name version))
     (sha256
      (base32 "1sn086kyvvcxabdlcqsq8f8r4lwf5x5pfyrnp3h185w3g0hi91wv"))))
   (build-system cmake-build-system)
   (arguments '(#:tests? #false)) ;no check target
   (home-page "https://github.com/cameron314/readerwriterqueue")
   (synopsis "A fast single-producer, single-consumer lock-free queue for C++")
   (description
    "This package provides a fast single-producer, single-consumer lock-free queue for C++11.")
   (license license:bsd-2)))

(define-public libstd
  (package
   (name "libstd")
   (version "0.0.2-dev")
   (source
    (origin
     (method git-fetch)
     (uri (git-reference
           (url "https://codeberg.org/v66v/libstd/")
           (commit "466144ff9373f25c42bfd54219bde4adc438cece")))
     (file-name (git-file-name name version))
     (sha256
      (base32 "0ndhbk8lx6930z43ywd0n013pbl6584jw3kzcin3z801y3ap2b04"))))
   (build-system gnu-build-system)
   (arguments
    (list #:phases
          #~(modify-phases %standard-phases
                           (delete 'configure)
                           (delete 'build)
                           (delete 'check)
                           (replace 'install
                                    (lambda* (#:key outputs #:allow-other-keys)
                                      (let ((out (assoc-ref outputs "out")))
                                        (invoke "make" "install"
                                                (string-append "DESTDIR=" out)))))
                           (add-after 'install 'check
                                      (lambda* (#:key outputs #:allow-other-keys)
                                        (let ((out (assoc-ref outputs "out")))
                                          (invoke "make" "check"
                                                  (string-append "DESTDIR=" out))))))))
   (native-inputs (append (list libtool cmocka pkg-config fontconfig)))
   (synopsis "A simple c standard library extension library.")
   (description "A simple c standard library extension library.")
   (home-page "https://github.com/v66v/std")
   (license license:lgpl3+)))

(package
 (name "SLIIPN-CS-AEDR")
 (version "0.0.1-git")
 (source
  (local-file "."
              "SLIIPN-CS-AEDR-checkout"
              #:recursive? #t
              #:select? vcs-file?))
 (build-system gnu-build-system)
 (arguments
  (list
   #:phases #~(modify-phases %standard-phases
                             (delete 'configure)
                             (delete 'check)
                             (replace 'install
                                      (lambda* (#:key outputs #:allow-other-keys)
                                        (let ((out (assoc-ref outputs "out")))
                                          (invoke "make" "install"
                                                  (string-append "DESTDIR=" out))))))))
 (inputs (list can-utils
               openssl
               readerwriterqueue
               concurrentqueue

               pkg-config
               gtk+
               `(,glib "bin")
               libxml2

               sqlite
               libstd

               parallel
               texlive-xetex
               texlive-graphics
               texlive-fontspec
               texlive-underscore

               pulseaudio

               netcat
               python
               python-parse
               python-matplotlib
               python-pycairo
               python-pygobject
               python-pandas))
 (synopsis "EDR/DA")
 (description "System Libraries for Immediate Inter-Process Notifications
Case-Study on Automotive Event Data Recorders")
 (home-page "https://github.com/v66v/SLIIPN-CS-AEDR")
 (license license:gpl3+))

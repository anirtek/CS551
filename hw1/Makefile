BASE = 			hw1

TARGETS =		$(BASE).html $(BASE).pdf

#Path definitions
PERL= 		/usr/bin/perl
UMT=		/home/zdu/projects/umt/umt
HTML2PS= 	html2ps
PS2PDF=		/usr/bin/ps2pdf

.PHONY:			clean

all:			$(TARGETS)


$(BASE).html:		$(BASE).umt
			$(UMT) $< >$@

$(BASE).pdf:		$(BASE).html
			$(HTML2PS) -N 0 -n $(BASE).html > $(BASE).ps
			$(PS2PDF) $(BASE).ps 
			rm -f $(BASE).ps

clean:
			rm -f $(TARGETS) *~

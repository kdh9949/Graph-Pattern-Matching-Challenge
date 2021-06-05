#!/bin/bash

pandoc report.md -o report.pdf \
	--pdf-engine=xelatex \
	--variable mainfont='NanumMyeongjo' \
	-V geometry:margin=2cm \
	-V fontsize=11pt

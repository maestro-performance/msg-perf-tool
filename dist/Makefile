VERSION=0.2.0

check:
	rpmlint -i dist/msg-perf-tool.spec

rpm-prep:
	mkdir -p ${HOME}/rpmbuild/SOURCES/
	tar --transform="s/\./msg-perf-tool-${VERSION}/" -cf ${HOME}/rpmbuild/SOURCES/msg-perf-tool-${VERSION}.tar.gz --exclude=./build --exclude=.git --exclude=*.bz ./ --gzip

rpm: rpm-prep
	rpmbuild -ba dist/msg-perf-tool.spec

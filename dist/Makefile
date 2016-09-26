VERSION=0.1.0

rpm-prep:
	mkdir -p ${HOME}/rpmbuild/SOURCES/
	tar --transform="s/\./msg-perf-tool-${VERSION}/" -cf ${HOME}/rpmbuild/SOURCES/msg-perf-tool-${VERSION}.tar.gz --exclude=./build --exclude=.git --exclude=*.bz ./ --gzip

fedora-rpm: rpm-prep
	rpmbuild -ba dist/fc-rpm.spec

rhel-rpm: rpm-prep
	rpmbuild -ba dist/rhel-rpm.spec

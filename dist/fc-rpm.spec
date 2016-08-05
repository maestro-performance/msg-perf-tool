%global _enable_debug_package 0
%global debug_package %{nil}

Summary:            Messaging Performance Tool
Name:               msg-perf-tool
Version:            0.1.0
Release:            1%{?dist}
License:            Apache v2
Group:              Development/Tools
Source:             msg-perf-tool-%{version}.tar.gz
URL:                https://github.com/orpiske/msg-perf-tool
BuildRequires:      cmake
BuildRequires:      make
BuildRequires:      gcc
BuildRequires:      gcc-c++
BuildRequires:      qpid-proton-c-devel
BuildRequires:      apr-devel
BuildRequires:      apr-util-devel
BuildRequires:      litestomp-devel
Requires:           qpid-proton-c
Requires:           python
Requires:           python-requests


%description
A tool for measuring messaging system performance

%prep
%autosetup -n msg-perf-tool-%{version}

%build
mkdir build && cd build
cmake -DSTOMP_SUPPORT=ON -DAMQP_SUPPORT=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr ..
make

%install
cd build
make install

%files
%doc README.md LICENSE
%{_bindir}/*
%{_libdir}/*
%{_datadir}/*


%changelog
* Fri Aug 05 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160805
- Version 0.1.0 release
- Removed self generated data using gnuplot
- Removed the log parser
- Performance data is saved straight to CSV instead of being parsed later
- Added a loader to load data to ElasticSearch database
- Cleaned up the logs
- Added stomp support
- Improved support for Raspberry PI
- Minor fixes for memory management and file usage

* Wed Jun 15 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160615
- Improved the runner script with additional information about the test execution
- Small fixes for validating input parameters

* Thu Jun 09 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160609
- Initial release

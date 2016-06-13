%global _enable_debug_package 0
%global debug_package %{nil}

Summary:            Messaging Performance Tool
Name:               msg-perf-tool
Version:            0.0.1
Release:            3%{?dist}
License:            Apache v2
Group:              Development/Tools
Source:             msg-perf-tool-%{version}.tar.gz
URL:                https://github.com/orpiske/msg-perf-tool
BuildRequires:      cmake
BuildRequires:      make
BuildRequires:      gcc
BuildRequires:      gcc-c++
BuildRequires:      qpid-proton-c-devel
Requires:           qpid-proton-c
Requires:           gnuplot
Requires:           python


%description
A tool for measuring messaging system performance

%prep
%setup -n msg-perf-tool-%{version}

%build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=%{buildroot}/usr ..
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
* Thu Jun 09 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160609
- Initial release

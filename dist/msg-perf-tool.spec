# Debug info package generation currently breaks the RPM build
%global _enable_debug_package 0
%global debug_package %{nil}

Summary:            Messaging Performance Tool
Name:               msg-perf-tool
Version:            0.2.1
Release:            4%{?dist}
License:            Apache v2
Source:             msg-perf-tool-%{version}.tar.gz
URL:                https://github.com/orpiske/msg-perf-tool.git
BuildRequires:      cmake
BuildRequires:      make
BuildRequires:      gcc >= 4.8.0
BuildRequires:      gcc-c++
BuildRequires:      qpid-proton-c-devel
BuildRequires:      apr-devel
BuildRequires:      apr-util-devel
BuildRequires:      litestomp-devel
BuildRequires:      paho-c-devel
BuildRequires:      bmic-devel
BuildRequires:      hdr-histogram-c-devel
BuildRequires:      msgpack-devel
BuildRequires:      readline-devel
BuildRequires:      libuuid-devel
BuildRequires:      zlib-devel
BuildRequires:      uriparser-devel
Requires:           python

%description
A tool for measuring messaging system performance for AMQP, STOMP and MQTT messaging protocols

%prep
%autosetup -n msg-perf-tool-%{version}

%build
mkdir build && cd build
%cmake -DSTOMP_SUPPORT=ON -DAMQP_SUPPORT=ON -DMQTT_SUPPORT=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_USER_C_FLAGS="-fPIC" ..
%make_build


%install
cd build
%make_install

%files
%doc README.md
%license LICENSE
%{_bindir}/*
%{_libdir}/*
%{_sysconfdir}/*
%{_prefix}/lib/systemd/*

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%changelog
* Fri Mar 01 2018 Otavio R. Piske <angusyoung@gmail.com> - 0.2.1-4
- Dummy version bump to try to fix COPR issues

* Fri Dec 15 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.1-3
- Fixed saving the protocol information on the test properties

* Fri Nov 24 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.1-2
- Incorrect help message for the data server

* Fri Nov 10 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.1-1
- Display throttling information on the CLI
- Fix incorrectly calculation of throttling
- Cleanup unused parameters
- Fix incorrect QOS for Maestro's LWT message
- Reduce logging of PING maestro message

* Thu Oct 12 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.1-0
- Fixed incorrect computation of throughput
- Abnormal disconnect message is now sent as a notification message
- Dump messaging API information in test.properties file
- Provide more relevant role information
- Fix Maestro CLI to avoid errors due to empy lines

* Thu Oct 12 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-7
- Update snapshot upon stop to prevent old data from remaining in the queue

* Wed Oct 04 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-6
- Fixes incorrect error cleanup in the workers

* Wed Sep 27 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-5
- Performance tunning: adjust the size of the IPC buffer
- Link to the last execution even if failed
- Flush all buffers prior to test start (brokerd)

* Wed Sep 27 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-4
- Use a dynamic wait time if using more than 10 children

* Wed Sep 27 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-3
- Incorrect evaluation of latency within the warm up period

* Mon Sep 25 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-2
- Prevent the test from failing if within the warm up period

* Wed Aug 30 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-1
- Bump rpm build number

* Sat Aug 26 2017 Otavio R. Piske <angusyoung@gmail.com> - 0.2.0-0
- Adjusted to match fedora packaging guidelines

* Mon Feb 27 2017 Otavio R. Piske <angusyoung@gmail.com> - 20170227
- Version 0.2.0 release
- Fixed a bug in the AMPQ setling mode
- Fixed a bug in the runner which passed incorrect parameters to the benchmark tools
- Added support for probes 
- Added support for recording broker JVM metrics
- Added experimental support for MQTT protocol
- Minor performance improvements on the benchmark tools
- Improved error handling
- Added a performance tuning mode to help guessing an adequate throtling value
- Adjusted the reporting data to reduce disk usage and DB loading time


* Fri Oct 14 2016 Otavio R. Piske <angusyoung@gmail.com> - 20161014
- Version 0.1.1 release
- Several bug fixes in the loader
- The loader now uses time-based index for greater performance on the UI
- The loader now adds additional mappings for all loaded index
- The loader now requires the test start time for additional operations


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
- Fixes an incorrect variable reference

* Wed Jun 15 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160615
- Improved the runner script with additional information about the test execution
- Small fixes for validating input parameters

* Thu Jun 09 2016 Otavio R. Piske <angusyoung@gmail.com> - 20160609
- Initial release

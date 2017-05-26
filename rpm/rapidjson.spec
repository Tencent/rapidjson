%global commit0 %{COMMIT_ID}
%global shortcommit0 %(c=%{commit0}; echo ${c:0:7})

Name:           rapidjson-devel
Version:        1.1.0
Release:	%{shortcommit0}%{dist}
Summary:	Websocketpp header package
Group:		Development/Libraries
License:	Proprietry
URL:      https://github.com/miloyip/rapidjson/
Source0:	rapidjson-%{COMMIT_ID}.tar.gz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}%{dist}-XXXXXX)
BuildRequires:	cmake, make, rpm-build

%description
Websocketpp development package.
For details, please refer to https://github.com/miloyip/rapidjson/


%define debug_package %{nil}


%prep
%setup -q -n rapidjson


%build
mkdir rpm_build_result
pushd rpm_build_result
cmake -DCMAKE_INSTALL_PREFIX=/usr ..
make
popd


%install
rm -rf %{buildroot}
pushd rpm_build_result
DESTDIR=%{buildroot} make install
popd


%clean
rm -rf %{buildroot}


%files
%defattr(0644,root,root,0755)
/usr

#%changelog

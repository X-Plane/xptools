Summary: X-Plane Scenery Tools
Name: xptools
Version: 9.06
Release: 1
License: MIT/X11
Group: Applications/Games
URL: http://scenery.x-plane.com/
Source: xptools.tar.bz2

%description
X-Plane Scenery Tools

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Provides: WED
Provides: DDSTool
Provides: DSFTool
Provides: MeshTool
Provides: ObjConverter
Provides: ObjView
Provides: RenderFarm

Requires: qt >= 4.5.0
BuildRequires: gcc >= 4.2
BuildRequires: qt-devel >= 4.5.0


%prep
%setup -n xptools

%build
make conf=release_test DDSTool DSFTool MeshTool ObjConverter \
ObjView RenderFarm WED

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/WED $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/DDSTool $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/DSFTool $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/MeshTool $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/ObjConverter $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/ObjView $RPM_BUILD_ROOT/usr/bin
cp $RPM_BUILD_DIR/xptools/build/Linux/release_test/RenderFarm $RPM_BUILD_ROOT/usr/bin

%clean
rm -rf $RPM_BUILD_ROOT
rm -rf $RPM_BUILD_DIR/xptools

%pre
%post

%preun
%postun

%files
%attr(0755,root,root) /usr/bin/WED
%attr(0755,root,root) /usr/bin/DDSTool
%attr(0755,root,root) /usr/bin/DSFTool
%attr(0755,root,root) /usr/bin/MeshTool
%attr(0755,root,root) /usr/bin/ObjConverter
%attr(0755,root,root) /usr/bin/ObjView
%attr(0755,root,root) /usr/bin/RenderFarm

import re

# This program is to aid making a release
# It patches a number of files that have version numbers and/or dates
# in them.
#
# configure.ac:6:AC_INIT([robodoc], [4.99.44])
# INSTALL.md:15:the official source distribution (robodoc-4.99.44.zip) you can build ROBODoc using:
# INSTALL.md:18:    unzip robodoc-4.99.44.zip
# INSTALL.md:19:    cd robodoc-4.99.44
# README.md:1:# ROBODoc Version 4.99.44 February 2021
# Docs/robodoc.1:14:.TH ROBODoc "1" "Feb 2021" "robodoc 4.99.44"
# Docs/manual.html


def read_all(path):
    lines = None
    with open(path, 'r') as inpf:
        lines = inpf.readlines()
    assert lines is not None
    return lines


def write_all(path, patched):
    with open(path, 'w') as outpf:
        for line in patched:
            outpf.write(line)


def patch_configure_ac(lines, new_version, new_month, new_year):
    patched = []
    for line in lines:
        is_ac_init = re.match("AC_INIT", line)
        if is_ac_init:
            line = "AC_INIT([robodoc], [{}])\n".format(new_version)
        patched.append(line)
    return patched


def patch_install_md(lines, new_version, new_month, new_year):
    patched = []
    for line in lines:
        has_version = re.search(r"robodoc-\d.\d\d.\d\d", line)
        if has_version:
            line = re.sub(
                    r"robodoc-\d.\d\d.\d\d",
                    "robodoc-{}".format(new_version),
                    line)
        patched.append(line)
    return patched


def patch_readme_md(lines, new_version, new_month, new_year):
    patched = []
    for line in lines:
        is_title = re.match(
                r'\# ROBODoc Version \d.\d\d.\d\d \S+ 2\d\d\d', line)
        if is_title:
            line = "# ROBODoc Version {} {} {}\n"
            line = line.format(new_version, new_month, new_year)
        patched.append(line)
    return patched


def patch_robodoc_1(lines, new_version, new_month, new_year):
    patched = []
    for line in lines:
        is_title = re.match(r'.TH ROBODoc "1" "\S+ 2\d\d\d"', line)
        if is_title:
            line = '.TH ROBODoc "1" "{} {}" "@PACKAGE_STRING@"'
            line = line.format(new_month, new_year)
            line = line + "\n"
        patched.append(line)
    return patched


def patch_manual_html(lines, new_version, new_month, new_year):
    patched = []
    for line in lines:
        line = re.sub(r'\d\.\d\d\.\d\d', new_version, line)
        patched.append(line)
    return patched


if __name__ == "__main__":
    new_version = '4.99.45'
    new_month = 'Feb'
    new_year = '2021'

    file_patchers = {
            'configure.ac': patch_configure_ac,
            'INSTALL.md': patch_install_md,
            'README.md':  patch_readme_md,
            'Docs/robodoc.1.in': patch_robodoc_1,
            'Docs/manual.html': patch_manual_html
            }

    for path in file_patchers:
        lines = read_all(path)
        patched_lines = file_patchers[path](
                lines, new_version, new_month, new_year)
        write_all(path, patched_lines)

# --------------- end of file -----------------------------------------

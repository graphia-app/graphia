#! /bin/bash
#
# Copyright © 2013-2024 Graphia Technologies Ltd.
#
# This file is part of Graphia.
#
# Graphia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Graphia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
#

use HTML::TextToHTML;
use HTML::FormatRTF;
use File::Basename;

my $dirname = dirname(__FILE__);

my $txt_file = "$dirname/../../../../LICENSE";
my $html_file = "$dirname/LICENSE.html";

my $conv = new HTML::TextToHTML();

$conv->txt2html(infile=>[$txt_file], outfile=>$html_file,
    title=>"License", mail=>1);

my $rtf_file = "$dirname/LICENSE.rtf";
open(RTF, ">$rtf_file")
 or die "Can't write-open $rtf_file: $!\nAborting";

print RTF HTML::FormatRTF->format_file("$html_file");
close(RTF);

############################################################################
#       date_release.awk - script to replace date and release tag of a docbook file
#                            -------------------
#   begin                : Mon May 14 2007
#   copyright            : (C) 2007 by Thomas Eschenbacher
#   email                : Thomas.Eschenbacher@gmx.de
############################################################################
#
############################################################################
#                                                                          #
#    This program is free software; you can redistribute it and/or modify  #
#    it under the terms of the GNU General Public License as published by  #
#    the Free Software Foundation; either version 2 of the License, or     #
#    (at your option) any later version.                                   #
#                                                                          #
############################################################################

{ 
    sub(/<date>.*<\/date>/, "<date>"newdate"</date>", $0);
    sub(/<release>.*<\/release>/, "<release>"newrelease"</release>", $0);
    print; 
}

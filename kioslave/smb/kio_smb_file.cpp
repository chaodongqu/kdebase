////////////////////////////////////////////////////////////////////////////
//
// Project:     SMB kioslave for KDE2
//
// File:        kio_smb_file.cpp
//
// Abstract:    member function implementations for SMBSlave that deal with
//              SMB file access
//
// Author(s):   Matthew Peterson <mpeterson@caldera.com>
//
//---------------------------------------------------------------------------
//
// Copyright (c) 2000  Caldera Systems, Inc.
//
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2.1 of the License, or
// (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Lesser General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program; see the file COPYING.  If not, please obtain
//     a copy from http://www.gnu.org/copyleft/gpl.html
//
/////////////////////////////////////////////////////////////////////////////


#include "kio_smb.h"
#include "kio_smb_internal.h"

#include <kmimetype.h>

//===========================================================================
void SMBSlave::get( const KURL& kurl )
{
    char        buf[MAX_XFER_BUF_SIZE];
    int         filefd          = 0;
    ssize_t     bytesread       = 0;
    // time_t      curtime         = 0;
    time_t      lasttime        = 0;
    time_t      starttime       = 0;
    KIO::filesize_t totalbytesread  = 0;
    QByteArray  filedata;
    SMBUrl      url;

    kdDebug(KIO_SMB) << "SMBSlave::get on " << kurl << endl;

    // check (correct) URL
    KURL kvurl = checkURL(kurl);
    // if URL is not valid we have to redirect to correct URL
    if (kvurl != kurl) {
        redirection(kvurl);
        finished();
        return;
    }

    if(!auth_initialize_smbc())
        return;


    // Stat
    url = kurl;
    if(cache_stat(url,&st) == -1 )
    {
        if ( errno == EACCES )
           error( KIO::ERR_ACCESS_DENIED, url.prettyURL());
        else
           error( KIO::ERR_DOES_NOT_EXIST, url.prettyURL());
        return;
    }
    if ( S_ISDIR( st.st_mode ) ) {
        error( KIO::ERR_IS_DIRECTORY, url.prettyURL());
        return;
    }

    // Set the total size
    totalSize( st.st_size );

    // Open and read the file
    filefd = smbc_open(url.toSmbcUrl(),O_RDONLY,0);
    if(filefd >= 0)
    {
        if(buf)
        {
	    bool isFirstPacket = true;
            lasttime = starttime = time(NULL);
            while(1)
            {
                bytesread = smbc_read(filefd, buf, MAX_XFER_BUF_SIZE);
                if(bytesread == 0)
                {
                    // All done reading
                    break;
                }
                else if(bytesread < 0)
                {
                    error( KIO::ERR_COULD_NOT_READ, url.prettyURL());
                    return;
                }

                filedata.setRawData(buf,bytesread);
		if (isFirstPacket)
		{
                    // We need a KMimeType::findByNameAndContent(filename,data)
                    // For now we do: find by extension, and if not found (or extension not reliable)
                    // then find by content.
                    bool accurate = false;
                    KMimeType::Ptr mime = KMimeType::findByURL( kurl, st.st_mode, false, true, &accurate );
                    if ( !mime || mime->name() == KMimeType::defaultMimeType()
                         || !accurate )
                    {
                        KMimeType::Ptr p_mimeType = KMimeType::findByContent(filedata);
                        if ( p_mimeType && p_mimeType->name() != KMimeType::defaultMimeType() )
                            mime = p_mimeType;
                    }
		    mimeType(mime->name());
		    isFirstPacket = false;
		}
                data( filedata );
                filedata.resetRawData(buf,bytesread);

                // increment total bytes read
                totalbytesread += bytesread;

		processedSize(totalbytesread);
            }
        }

        smbc_close(filefd);
        data( QByteArray() );
        processedSize(static_cast<KIO::filesize_t>(st.st_size));

    }
    else
    {
          error( KIO::ERR_CANNOT_OPEN_FOR_READING, url.prettyURL());
	  return;
    }

    finished();
}


//===========================================================================
void SMBSlave::put( const KURL& kurl,
                    int permissions,
                    bool overwrite,
                    bool resume )
{

    void *buf;
    size_t bufsize;

    m_current_url = kurl;

    int         filefd;
    bool        exists;
    mode_t      mode;
    QByteArray  filedata;

    kdDebug(KIO_SMB) << "SMBSlave::put on " << kurl << endl;


    exists = (cache_stat(m_current_url, &st) != -1 );
    if ( exists &&  !overwrite && !resume)
    {
        if (S_ISDIR(st.st_mode))
        {
            kdDebug(KIO_SMB) << "SMBSlave::put on " << kurl <<" already isdir !!"<< endl;
            error( KIO::ERR_DIR_ALREADY_EXIST,  m_current_url.prettyURL());
        }
        else
        {
            kdDebug(KIO_SMB) << "SMBSlave::put on " << kurl <<" already exist !!"<< endl;
            error( KIO::ERR_FILE_ALREADY_EXIST, m_current_url.prettyURL());
        }
        return;
    }

    if (exists && !resume && overwrite)
    {
        kdDebug(KIO_SMB) << "SMBSlave::put exists try to remove " << m_current_url.toSmbcUrl()<< endl;
        //   remove(m_current_url.url().local8Bit());
    }


    if (resume)
    {
        // append if resuming
        kdDebug(KIO_SMB) << "SMBSlave::put resume " << m_current_url.toSmbcUrl()<< endl;
        filefd = smbc_open(m_current_url.toSmbcUrl(), O_RDWR, 0 );
        smbc_lseek(filefd, 0, SEEK_END);
    }
    else
    {
        if (permissions != -1)
        {
            mode = permissions | S_IWUSR | S_IRUSR;
        }
        else
        {
            mode = 600;//0666;
        }

        kdDebug(KIO_SMB) << "SMBSlave::put NO resume " << m_current_url.toSmbcUrl()<< endl;
        filefd = smbc_open(m_current_url.toSmbcUrl(), O_CREAT | O_TRUNC | O_WRONLY, mode);
    }

    if ( filefd < 0 )
    {
        if ( errno == EACCES )
        {
            kdDebug(KIO_SMB) << "SMBSlave::put error " << kurl <<" access denied !!"<< endl;
            error( KIO::ERR_WRITE_ACCESS_DENIED, m_current_url.prettyURL());
        }
        else
        {
            kdDebug(KIO_SMB) << "SMBSlave::put error " << kurl <<" can not open for writing !!"<< endl;
            error( KIO::ERR_CANNOT_OPEN_FOR_WRITING, m_current_url.prettyURL());
        }
        finished();
        return;
    }

    // Loop until we got 0 (end of data)
    while(1)
    {
        kdDebug(KIO_SMB) << "SMBSlave::put request data "<< endl;
        dataReq(); // Request for data
        kdDebug(KIO_SMB) << "SMBSlave::put write " << m_current_url.toSmbcUrl()<< endl;

        if (readData(filedata) <= 0)
        {
            kdDebug(KIO_SMB) << "readData <= 0" << endl;
            break;
        }
        kdDebug(KIO_SMB) << "SMBSlave::put write " << m_current_url.toSmbcUrl()<< endl;
	buf = filedata.data();
	bufsize = filedata.size();
        int size = smbc_write(filefd, buf, bufsize);
        if ( size < 0)
        {
            kdDebug(KIO_SMB) << "SMBSlave::put error " << kurl <<" could not write !!"<< endl;
            error( KIO::ERR_COULD_NOT_WRITE, m_current_url.prettyURL());
            finished();
            return;
        }
        kdDebug(KIO_SMB ) << "wrote " << size << endl;
    }
    kdDebug(KIO_SMB) << "SMBSlave::put close " << m_current_url.toSmbcUrl()<< endl;

    if(smbc_close(filefd))
    {
        kdDebug(KIO_SMB) << "SMBSlave::put on " << kurl <<" could not write !!"<< endl;
        error( KIO::ERR_COULD_NOT_WRITE, m_current_url.prettyURL());
        finished();
        return;
    }

    // set final permissions, if the file was just created
    if ( permissions != -1 && !exists )
    {
        // TODO: did the smbc_chmod fail?
        // TODO: put in call to chmod when it is working!
        // smbc_chmod(url.toSmbcUrl(),permissions);
    }

    // We have done our job => finish
    finished();
}





#ifndef SFTP_DIR_H
#define SFTP_DIR_H

#include <libssh2_sftp.h>
#include "SshException.h"

// Forward declaration to avoid recursive includes
class SFtpChannel;

/**
 * @brief The SFtpDir class A MUSE class that eases the interaction with the
 * libssh2 library. The methods within this class call on a series of libssh2
 * methods in order to accomplish their respective tasks.
 */
class SFtpDir {
public:
    /**
     * @brief SFtpDir Creates an instance of SFtpDir so that operations may be
     * performed on a directory that resides within a remote file system. This
     * constructor should only be called if dir already exists. The constructor
     * does NOT create a new directory if it does not exist in the file system.
     * @param sftp The SFtpChannel that communications will be sent through.
     * @param dir The path to the directory this instance will be referring to.
     */
    SFtpDir(SFtpChannel& sftp, const QString& dir) throw (const SshException &);

    ~SFtpDir() throw ();

    /**
     * @brief getInfo Gets information about the directory, namely, the size
     * of the contents of the directory, the timestamp of the directory, and
     * the permissions surrounding the directory.
     * @param size A reference that will receive the size of the contents of
     * this directory.
     * @param timestamp A reference that will receive the timestamp of this
     * directory.
     * @param flags A reference that will receive the permissions set on this
     * directory.
     */
    void getInfo(long& size, long& timestamp, int& flags)
    throw (const SshException &);

    /**
     * @brief getEntry Attempts to read the attributes of file and returns true
     * if the attributes were successfully read.
     * @param file The path to the file to be read.
     * @param sizeA  reference that will receive the size of the contents of
     * the file.
     * @param timestamp A reference that will receive the timestamp of the
     * file.
     * @param flags reference that will receive the permissions set on the
     * file.
     * @return True if the file could be read, otherwise false.
     */
    bool getEntry(QString& file, long& size, long& timestamp, int& flags)
    throw (const SshException &);

    /**
     * @brief good Returns whether or not this SFtpDir is in good condition, a
     * state maintained by the goodFlag.
     * @return The state of this SFtoDir.
     */
    bool good() const { return goodFlag; }

    /**
     * @brief getPath Gets the path to this SFtpDir.
     * @return A QString representation of the path to this SFtpDir.
     */
    QString getPath() const { return dir; }

private:
    SFtpChannel& sftp;
    QString dir;
    bool goodFlag;
    LIBSSH2_SFTP_HANDLE *sftpDir;
};

#endif // SFTP_DIR_H

#ifndef REMOTEFSHELPER_H
#define REMOTEFSHELPER_H

#include "FSHelperCommon.h"
#include "SFtpChannel.h"

/**
 * @brief The RemoteFSHelper class A class that helps with file
 * operations on remote file systems. This class is part of the
 * CustomFileDialog system. This class will use the ssh2 library
 * for performing operations on the remote file system.
 */
class RemoteFSHelper : public FSHelperCommon {
    Q_OBJECT
public:
    /**
     * @brief RemoteFSHelper The constructor for the RemoteFSHelper
     * class.
     * @param ssh Pointer to the ssh socket that has been established
     * with the remote computer.
     * @param deleteSocket Whether or not the socket should be
     * deleted when this class is deleted.
     */
    RemoteFSHelper(SshSocket *ssh, const bool deleteSocket = false);
    ~RemoteFSHelper();

    /**
     * @brief getEntries Gets a listing of entries at the specified
     * directory.
     * @param dir The directory to get the listing of entries from.
     * @return The listing of entries.
     */
    const FSEntryList* getEntries(const FSEntry& dir) const;

    /**
     * @brief getRoot Gets the root of the current file system.
     * @return The FSEntry of the root in the current file system.
     */
    const FSEntry&     getRoot() const;

    /**
     * @brief getHomePath Gets the path to the home directory on
     * this local file system.
     * @return A QString representing the path to the home directory
     */
    QString  getHomePath()  const;

    /**
     * @brief getSeparator Gets the character used as a path separator
     * for the file system. In Qt, this is always "/", regardless of
     * the operating system.
     * @return The QString representation of the path separator.
     */
    QString  getSeparator() const;

    /**
     * @brief getColumns Gets the number of columns to use for
     * display in the CustomFileDialog.
     * @return The number of columns to use for displaying file and
     * directory information.
     */
    int      getColumns()   const { return 3; }

protected:
    /**
     * @brief populateCache Populates the cache if it does not already
     * have the FSEntry given as a parameter in the cache. The method
     * informs the caller of the method if the cache has been populated.
     * @param parentDir The FSEntry to populate the cache with.
     * @return A boolean indicating if parentDir is in the cache after
     * calling this method.
     */
    bool populateCache(const FSEntry& parentDir) const;
    void addEntries(const FSEntry& parentDir, SFtpDir& dir, FSEntryList& list) const
    throw (const SshException &);

    /**
     * @brief addDir Adds the given directory to the listing of FSEntries.
     * This method throws an SshException if an error occurs.
     * @param parentDir The FSEntry of the directory to add to the list.
     * @param dir The SFtpDir of the directory on the remote server.
     * @param list The list dir will be added to.
     */
    void addDir(const FSEntry &parentDir, SFtpDir &dir, FSEntryList& list) const
    throw (const SshException &);

    /**
     * @brief convert Converts the file flags on the remote system
     * into the set of file flags implemented within the MUSE
     * implementation of the FileSystem (FS) classes.
     * @param sftpFlags the sftp flags to be converted into the MUSE
     * implementation.
     * @return The MUSE representaiton of the sftp flags.
     */
    int  convert(const int sftpFlags) const;

protected slots:
    /**
     * @brief moveToThread Moves the given ssh socket to a given
     * QThread.
     * @param thread The thread that the SshSocket will be moved to.
     * @param ssh The SshSocket to be moved to a different thread.
     */
    void moveToThread(QThread* thread, SshSocket *ssh);

signals:
    void needSocket(QThread *thread, SshSocket *ssh);

private:
    SshSocket* const ssh;
    const bool deleteSocket;
    const FSEntry root;
};

#endif // REMOTEFSHELPER_H

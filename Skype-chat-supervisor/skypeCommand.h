#ifndef _skypeCommand_H
#define _skypeCommand_H

/***************************************************************
 * skypeCommand.h
 * foundation by:
 * @Author:      Jonathan Verner (jonathan.verner@matfyz.cz)
 * @License:     GPL v2.0 or later
 * @Last Change: 2008-04-25.
 ***************************************************************/

#include <QtCore/QString>
#include <QtCore/QStringList>

enum skypeResponses { SK_OK, SK_ERROR, SK_INFO, SK_READY_TO_READ, SK_DATA, SK_STREAMS, SK_UNKNOWN, SK_NO_COMMAND, SK_STATUS, SK_ECHO, SK_END_OF_DATA, SK_CLOSE_STREAM, SK_PARSE_ERROR, SK_PING, SK_CONTACT_LIST, SK_CURRENT_USER, SK_CONTACT_STATUS, SK_USER_STATUS, SK_CHATMESSAGE };

class skypeResponse {
	public:
                enum contactStatus { offline, online, away, other };
                enum chatMessageStatus { SK_CMS_RECEIVED, SK_CMS_SENDING, SK_CMS_SENT, SK_CMS_READ };

	private:
		enum skypeResponses Type;
		QString Msg;

		int StreamNum, ProtocolNum, ErrorCode;
		QString ContactName, AppName;
		QString ErrorMsg;
		QString ResponseID;
		QByteArray Data;
		QStringList Contacts;
		contactStatus cStatus;

                QString ChatMessageId;
                QString ChatMessageProperty;
                enum chatMessageStatus mStatus;

		void clear();

	public:
		skypeResponse();
		bool parse(const QString &msg);
		QString responseID() {return ResponseID;};
		int protocolNum() {return ProtocolNum;};
		int streamNum() {return StreamNum;};
		QString appName() { return AppName; };
		QString contactName() {return ContactName;};
		QByteArray data() {return Data;};
		QStringList getContacts() { return Contacts; };
		contactStatus getStatus() { return cStatus; };
		int errorCode() {return ErrorCode;};
		QString errorMsg() {return ErrorMsg;};
		enum skypeResponses type() {return Type;};
		QString streamID(); // returns contactName:streamNum
		QString _debugState();

                QString getChatMessageId(){return ChatMessageId;};
                QString getChatMessageProperty(){return ChatMessageProperty;};
                QString getChatMessageContent(){return Msg;};
                //QString getMessage(){return ChatMessageId;};
                enum chatMessageStatus getMStatus() {return mStatus;};

};

class skypeCommand {
	private:
		static int ID;
	public:
		static QString CONNECT_TO_SKYPE(QString AppName);
		static QString CREATE(QString appName);
		static QString CONNECT(QString appName, QString contactName);
		static QString GET_CONTACT_LIST();
		static QString GET_CONTACT_STATUS(QString contactName);
		static QString CALL_CONTACT( QString contactName );
		static QString WRITE(QString appName, QString contactName, int streamNum, QByteArray data);
		static QString READ(QString appName, QString contactName, int streamNum);
		static QString DISCONNECT(QString appName, QString contactName, int streamNum);
		static QString DELETE(QString AppObject);
		static QString PING();
		static QString PROTOCOL(int protocolNum);
		static QString prependID(QString command, QString myID);
		static QString prependID(QString command);
		static QString getID(QString command);
		static QString streamID(QString contactName, int streamNum);
		static void    removeID(QString &msg);

                static QString GET_CHATMESSAGE_PROPERTY(QString ChatMessageId, QString property);
                static QString SEND_CHATMESSAGE(QString ChatMessageName, QString message);
};





#endif /* _skypeCommand_H */

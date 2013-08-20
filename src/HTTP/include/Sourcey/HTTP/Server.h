//
// LibSourcey
// Copyright (C) 2005, Sourcey <http://sourcey.com>
//
// LibSourcey is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// LibSourcey is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//


#ifndef SOURCEY_HTTP_Server_H
#define SOURCEY_HTTP_Server_H


#include "Sourcey/Base.h"
#include "Sourcey/Logger.h"
#include "Sourcey/Net/Socket.h"
#include "Sourcey/HTTP/Connection.h"
#include "Sourcey/HTTP/Request.h"
#include "Sourcey/HTTP/Response.h"
#include "Sourcey/HTTP/Parser.h"
#include "Sourcey/Timer.h"

	
namespace scy { 
namespace http {


class Server;
class ServerResponder;
class ServerConnection: public Connection
{
public:
    ServerConnection(Server& server, const net::Socket& socket);
	
	//virtual bool send();
		/// Sends the HTTP response
	
	virtual void close();
		/// Closes the HTTP connection
	
protected:
    virtual ~ServerConnection();
		
	virtual void onHeaders();
	virtual void onPayload(const MutableBuffer& buffer);
	virtual void onMessage();
	virtual void onClose();
				
	Server& server();

	http::Message* incomingHeader();
	http::Message* outgoingHeader();

	//
	/// Server callbacks
	void onServerShutdown(void*);
	
protected:
	Server& _server;
	ServerResponder* _responder;	
	bool _upgrade;
	bool _requestComplete;
};


typedef std::vector<ServerConnection*> ServerConnectionList;

	
// -------------------------------------------------------------------
//
class ServerAdapter: public ConnectionAdapter
{
public:
    ServerAdapter(ServerConnection& connection) : 
		ConnectionAdapter(connection, HTTP_REQUEST)
	{
	}
};


// -------------------------------------------------------------------
//
class ServerResponder
	/// The abstract base class for HTTP ServerResponders 
	/// created by HTTP Server.
	///
	/// Derived classes must override the handleRequest() method.
	///
	/// A new HTTPServerResponder object will be created for
	/// each new HTTP request that is received by the HTTP Server.
	///
{
public:
	ServerResponder(ServerConnection& connection) : 
		_connection(connection)
	{
	}

	virtual ~ServerResponder() {}

	virtual void onHeaders(Request& /* request */) {}
	virtual void onPayload(const MutableBuffer& /* body */) {}
	virtual void onRequest(Request& /* request */, Response& /* response */) {}
	virtual void onClose() {};

	ServerConnection& connection()
	{
		return _connection;
	}
		
	Request& request()
	{
		return _connection.request();
	}
	
	Response& response()
	{
		return _connection.response();
	}

protected:
	ServerConnection& _connection;

private:
	ServerResponder(const ServerResponder&); // = delete;
	ServerResponder(ServerResponder&&); // = delete;
	ServerResponder& operator=(const ServerResponder&); // = delete;
	ServerResponder& operator=(ServerResponder&&); // = delete;
};


// -------------------------------------------------------------------
//
class ServerResponderFactory
	/// This implementation of a ServerResponderFactory
	/// is used by HTTPServer to create ServerResponder objects.
{
public:
	virtual ServerResponder* createResponder(ServerConnection& connection) = 0;
		/// Factory method for instantiating the ServerResponder
		/// instance using the given ServerConnection.
};


// -------------------------------------------------------------------
//
class Server
	/// DISCLAIMER: This HTTP server is not intended to be standards 
	/// compliant. It was created to be a fast (nocopy where possible)
	/// solution for streaming video to web browsers.
	///
	/// TODO: 
	/// - SSL Server
	/// - Enable responders (controllers?) to be instantiated via
	///    registered routes.
{
public:
	ServerConnectionList connections;
	ServerResponderFactory* factory;
	net::TCPSocket socket;
	net::Address address;
	//Timer timer;

	Server(short port, ServerResponderFactory* factory);
	virtual ~Server();
	
	void start();
	void shutdown();

	UInt16 port();	

	NullSignal Shutdown;

protected:	
	ServerConnection* createConnection(const net::Socket& sock);
	ServerResponder* createResponder(ServerConnection& conn);

	virtual void addConnection(ServerConnection* conn);
	virtual void removeConnection(ServerConnection* conn);

	void onAccept(void* sender, const net::TCPSocket& sock);
	void onClose(void* sender);

	friend class ServerConnection;
};


// ---------------------------------------------------------------------
//
class BadRequestHandler: public ServerResponder
{
public:
	BadRequestHandler(ServerConnection& connection) : 		
		ServerResponder(connection)
	{		
	}

	void onRequest(Request&, Response& response)
	{
		response.setStatusAndReason(http::StatusCode::BadRequest);
		connection().sendHeader();
		connection().close();
	}
};


} } // namespace scy::http


#endif




/*
// ---------------------------------------------------------------------
//
class FlashPolicyConnectionHook: public ServerResponder
{
public:
	Poco::Net::TCPServerConnection* createConnection(const Poco::Net::StreamSocket& socket, const std::string& rawRequest)
	{		
		try 
		{			
			if (rawRequest.find("policy-file-request") != std::string::npos) {
				traceL("HTTPStreamingRequestHandlerFactory") << "send Flash Crossdomain XMLSocket Policy" << std::endl;
				return new Net::FlashPolicyRequestHandler(socket, false);
			}
			else if (rawRequest.find("crossdomain.xml") != std::string::npos) {
				traceL("HTTPStreamingRequestHandlerFactory") << "send Flash Crossdomain HTTP Policy" << std::endl;
				return new Net::FlashPolicyRequestHandler(socket, true);
			}			
		}
		catch (std::exception&Exception& exc)
		{
			LogError("ServerConnectionHook") << "Bad Request: " << exc.what()/message()/ << std::endl;
		}	
		return nullptr;
	};
};
*/
	
	/*
	void onTimer(void*)
	{
		ServerConnectionList conns = ServerConnectionList(connections);
		for (ServerConnectionList::iterator it = conns.begin(); it != conns.end();) {
			if ((*it)->closed()) {
				traceL("Server", this) << "Deleting connection: " << (*it) << std::endl;
				//delete *it;
				it = connections.erase(it);
			}
			else
				++it;
		}
	}
	*/
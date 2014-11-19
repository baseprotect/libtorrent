#ifndef BASEPROTECT_HPP
#define BASEPROTECT_HPP

#include <map>
#include <list>
#include <utility>

#ifndef Q_MOC_RUN
#include <boost/thread/mutex.hpp>
#include <boost/asio/ip/tcp.hpp>
#endif

#include <libtorrent/address.hpp>

using libtorrent::torrent;
using libtorrent::peer_request;
using libtorrent::sha1_hash;
using libtorrent::address;
using libtorrent::peer_id;
using libtorrent::piece_block;

using boost::asio::ip::tcp;

struct bp_peer_info
{
    bp_peer_info(peer_id id, tcp::endpoint remote,
                 int local_port_, std::string c)
        : pid(id),
          ip(remote.address()),
          port(remote.port()),
          local_port(local_port_),
          client(c)
    {}

    int port, local_port;
    address ip, local_ip;
    peer_id pid;

    std::string client;
};

struct bp_entry_t
{    
    bp_entry_t() {}
	
    bp_entry_t(boost::shared_ptr<torrent> t,
               bp_peer_info info,
               peer_request r,
               sha1_hash block_data_hash);

    bool operator < (const bp_entry_t& rhs)const
        { return pb < rhs.pb; }

    sha1_hash t_hash;

    peer_id pid;
    address ip, local_ip;
    size_t port, local_port;

    piece_block pb;
    sha1_hash p_hash, b_hash;

    size_t filesize;
    size_t piece_size;
    size_t block_size;

    std::string client;
    std::string filename;
    std::string path;
};

std::ostream& operator<<(std::ostream& str, const bp_entry_t& entry);

typedef std::pair<sha1_hash, int> key_type_t;
typedef std::map<key_type_t, std::set<bp_entry_t> > entry_map_t;

class bp_hits_manager
{
public:
    key_type_t make_key(boost::shared_ptr<torrent> t, int piece);

    void bp_add_peer(boost::shared_ptr<torrent> t,
                     bp_peer_info info,
                     peer_request r,
                     sha1_hash block_data_hash);

    std::list<bp_entry_t> get_entries();

    void valid_piece(boost::shared_ptr<torrent> t,  int piece);

    void invalid_piece(boost::shared_ptr<torrent> t,  int piece);

private:
    boost::mutex mutex;
    entry_map_t entry_map;
    std::list<bp_entry_t> valid_entries;
};

#endif // BASEPROTECT_HPP

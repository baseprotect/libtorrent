#include <map>
#include <utility>
#include <fstream>

#include <libtorrent/torrent.hpp>
#include <libtorrent/peer_info.hpp>
#include <libtorrent/peer_request.hpp>
#include <libtorrent/address.hpp>

#ifndef Q_MOC_RUN
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_guard.hpp>
#endif

#include "baseprotect.hpp"

using namespace libtorrent;

std::ostream& operator<<(std::ostream& str, const bp_entry_t& entry)
{
    str << entry.ip.to_string() << " "
        << entry.pid            << " "
        << entry.t_hash         << " "
        << entry.pb.piece_index << " "
        << entry.piece_size     << " "
        << entry.p_hash         << " "
        << entry.pb.block_index << " "
        << entry.b_hash         << " "
        << entry.block_size     << " "
        << std::endl;

    return str;
}

bp_entry_t::bp_entry_t(
        boost::shared_ptr<torrent> t,
        bp_peer_info info,
        peer_request r,
        sha1_hash block_data_hash)
{
    boost::lock_guard<boost::mutex> lock(mutex);

    pid = info.pid;
    client = info.client;

    ip = info.ip;
    port = info.port;

    local_port = info.local_port;
    local_ip = info.local_ip;

    pb = piece_block(r.piece, r.start / t->block_size());

    path = t->save_path();
    filename = t->name();
    t_hash = t->info_hash();

    const torrent_info& t_info = t->get_handle().get_torrent_info();
    filesize = t_info.total_size();

    b_hash = block_data_hash;
    p_hash = t_info.hash_for_piece(r.piece);

    piece_size = t_info.piece_size(r.piece);
    block_size = t->block_size();

    std::string pieces_file = "C:\\WORK\\pieces.log";
    if(libtorrent::exists(pieces_file))
    {
       std::ofstream pieces(pieces_file.c_str(), std::ios::app);
       pieces << *this << std::endl;
    }
}

key_type_t bp_hits_manager::make_key(boost::shared_ptr<torrent> t, int piece)
{
    return std::make_pair(t->info_hash(), piece);
}

void bp_hits_manager::bp_add_peer(
        boost::shared_ptr<torrent> t,
        bp_peer_info info,
        peer_request r,
        sha1_hash block_data_hash)
{
    entry_map[make_key(t, r.piece)].insert( bp_entry_t (t, info, r, block_data_hash) );
}

void bp_hits_manager::valid_piece(boost::shared_ptr<torrent> t, int piece)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    std::set<bp_entry_t> entries = entry_map[make_key(t, piece)];
    valid_entries.insert(valid_entries.begin(), entries.begin(), entries.end());
}

void bp_hits_manager::invalid_piece(boost::shared_ptr<torrent> t, int piece)
{
    boost::lock_guard<boost::mutex> lock(mutex);
    entry_map.erase(make_key(t, piece));
}

std::list<bp_entry_t> bp_hits_manager::get_entries()
{
    boost::lock_guard<boost::mutex> lock(mutex);
    return std::move(valid_entries);
}

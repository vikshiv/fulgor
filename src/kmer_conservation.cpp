#include "include/index.hpp"
#include "external/sshash/include/streaming_query.hpp"

namespace fulgor {

template <typename ColorSets>
void index<ColorSets>::kmer_conservation(
    std::string const& sequence,
    std::vector<kmer_conservation_triple>& kmer_conservation_info) const  //
{
    constexpr uint64_t invalid = uint64_t(-1);

    if (sequence.length() < m_k2u.k()) return;

    kmer_conservation_info.clear();
    sshash::streaming_query<kmer_type, true> query(&m_k2u);
    query.reset();
    const uint64_t num_kmers = sequence.length() - m_k2u.k() + 1;
    kmer_conservation_triple kct = {0, 0, 0};
    uint64_t prev_color_set_id = invalid;

    auto push_triple = [&]() {
        if (prev_color_set_id != invalid) {
            assert(kct.num_kmers != 0);
            kct.color_set_id = color_set(prev_color_set_id).size();
            kmer_conservation_info.push_back(kct);
        }
    };

    for (uint64_t i = 0; i != num_kmers; ++i) {
        char const* kmer = sequence.data() + i;
        auto answer = query.lookup_advanced(kmer);

        if (answer.kmer_id != sshash::constants::invalid_uint64) {  // kmer is positive

            uint64_t color_set_id = u2c(answer.contig_id);
            if (prev_color_set_id != color_set_id) {
                push_triple();
                kct.num_kmers = 0;
                kct.start_pos_in_query = i;
            }

            kct.num_kmers += 1;
            prev_color_set_id = color_set_id;

        } else {  // kmer is negative
            push_triple();
            prev_color_set_id = invalid;
        }
    }

    // push last one if we have to
    push_triple();
}

}  // namespace fulgor

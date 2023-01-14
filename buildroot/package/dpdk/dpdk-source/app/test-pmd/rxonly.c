/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2010-2014 Intel Corporation
 */

#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <inttypes.h>

#include <sys/queue.h>
#include <sys/stat.h>
#include <gem5/m5ops.h>

#include <rte_common.h>
#include <rte_byteorder.h>
#include <rte_log.h>
#include <rte_debug.h>
#include <rte_cycles.h>
#include <rte_memory.h>
#include <rte_memcpy.h>
#include <rte_launch.h>
#include <rte_eal.h>
#include <rte_per_lcore.h>
#include <rte_lcore.h>
#include <rte_atomic.h>
#include <rte_branch_prediction.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_interrupts.h>
#include <rte_pci.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_string_fns.h>
#include <rte_ip.h>
#include <rte_udp.h>
#include <rte_net.h>
#include <rte_flow.h>

#include "testpmd.h"
volatile char flag;

/*
 * Received a burst of packets.
 */
static void
pkt_burst_receive(struct fwd_stream *fs)
{
	struct rte_mbuf  *pkts_burst[MAX_PKT_BURST];
	struct rte_mbuf  *mb;
	uint16_t nb_rx;
	uint16_t i;
	uint64_t start_tsc = 0;

	get_start_cycles(&start_tsc);

	/*
	 * Receive a burst of packets.
	 */
	nb_rx = rte_eth_rx_burst(fs->rx_port, fs->rx_queue, pkts_burst,
				 nb_pkt_per_burst);
	inc_rx_burst_stats(fs, nb_rx);
	if (unlikely(nb_rx == 0))
		return;
	
	for (int i = 0; i < nb_rx; i++) {
		if (likely(i < nb_rx - 1)) 
			rte_prefetch0(rte_pktmbuf_mtod(pkts_burst[i + 1], void *));
	
		char *pkt_data;

		mb = pkts_burst[i];
		pkt_data = rte_pktmbuf_mtod(mb, char *);
		
		for (uint j = 0; j < mb->pkt_len; j++)
		{
			// Do something with data here
			if (pkt_data[j] == 255)
				flag = pkt_data[j];

			uint64_t timestamp;

			if (mb->pkt_len > 15)
				memcpy(&timestamp, &(pkt_data[8]), sizeof(uint64_t));
		}
	}

	fs->rx_packets += nb_rx;
	for (i = 0; i < nb_rx; i++)
		rte_pktmbuf_free(pkts_burst[i]);

	get_end_cycles(fs, start_tsc);
}

struct fwd_engine rx_only_engine = {
	.fwd_mode_name  = "rxonly",
	.port_fwd_begin = NULL,
	.port_fwd_end   = NULL,
	.packet_fwd     = pkt_burst_receive,
};

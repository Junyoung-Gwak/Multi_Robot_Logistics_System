#!/usr/bin/env python3
import rospy
from ssapang.msg import Locations, Coordinate, Move
import time

import heapq
from r_graph import r_graph


# def dijkstra(graph, start_node, destination_node):
def dijkstra(graph, start_node, destination_node, cantGoNode=None):
    # 시작 노드로부터의 거리 초기화
    dist = {node: float('inf') for node in graph}
    dist[start_node] = 0

    # 시작 노드를 큐에 삽입
    pq = [(0, start_node)]

    # 최단 경로를 저장할 딕셔너리 초기화
    prev = {node: None for node in graph}

    while pq:
        # 현재 가장 짧은 거리를 가진 노드 선택
        curr_dist, curr_node = heapq.heappop(pq)
        # 이미 처리된 노드라면 건너뜀
        if curr_dist > dist[curr_node]:
            continue
        # 현재 노드와 연결된 모든 노드를 순회
        for next_node in graph[curr_node]:
            if cantGoNode != None:  # 만약 지나면 안되는 노드가 있다면 통과
                if next_node == cantGoNode:
                    continue
            # 새로운 거리 계산
            new_dist = curr_dist + 1
            # 새로운 거리가 더 짧으면 업데이트
            if new_dist < dist[next_node]:
                dist[next_node] = new_dist
                prev[next_node] = [curr_node, graph.get(curr_node).get(next_node)]
                heapq.heappush(pq, (new_dist, next_node))
    # 도착 노드까지의 최단 경로 출력
    path = [[destination_node, graph.get(destination_node).get(destination_node)]]

    while path[-1][0] != start_node:
        path.append(prev[path[-1][0]])

    path.reverse()
    return path


def callback(msg):
    print(msg)
    try:
        shortest_dist = dijkstra(r_graph, msg.startNode, msg.endNode)
        print(shortest_dist)
        loc = Locations()
        for data in shortest_dist:
            coord = Coordinate()
            coord.QR = data[0]
            coord.deg = data[1] if data[1] != None else 0.0
            loc.location.append(coord)
        pub.publish(loc)
    except:
        print('error')


if __name__ == '__main__':
    rospy.init_node('path_pub')
    pub = rospy.Publisher('/path', Locations, queue_size=1)
    sub = rospy.Subscriber('/move', Move, callback)
    # while True:
    #     idx= input("픽업지점, 도착점을 입력하시오. :").split()
    #     start_point = 'B0101'
    #     # print(idx[0])
    #     try:
    #         shortest_dist1 = dijkstra(r_graph, start_point, idx[0])
    #         print(shortest_dist1)

    #         shortest_dist2 = dijkstra(r_graph, idx[0], idx[1])
    #         print(shortest_dist2)

    #         shortest_dist3 = dijkstra(r_graph, idx[1], start_point)
    #         print(shortest_dist3)
    #         # loc = Locations()
    #         # for i in range(len(shortest_dist)):
    #         #     coord = Coordinate()
    #         #     coord.QR = shortest_dist[i][0]
    #         #     coord.x = shortest_dist[i][2][1]
    #         #     coord.y = shortest_dist[i][2][0]
    #         #     coord.deg = shortest_dist[i][1]
    #         #     loc.location.append(coord)
    #         # pub.publish(loc)
    #     except:
    #         print('error')
    rospy.spin()
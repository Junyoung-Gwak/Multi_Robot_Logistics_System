#!/usr/bin/env python3
import rospy
from ssapang.msg import Locations, Coordinate, Move
from ssapang.srv import PathLen
import time
import copy

import heapq
from graph import graph
from node import node
burgerDontGo = ['BS0101', 'BS0103', 'BS0105', 'BS0108', 'BS0111', 'BS0113', 'BS0115']
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
            if cantGoNode!=None: #만약 지나면 안되는 노드가 있다면 통과
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
    
    path.append(copy.deepcopy(path[-1]))
    path.reverse()
    return path


def addCoordinates(arr):
    last = len(arr)-1
    arr[0].append(node.get(arr[0][0]))
    for i in range(1, last):
        arr[i].append(node.get(arr[i+1][0]))
        arr[i][0] = arr[i+1][0]
    arr[last].append([100,100])
    # arr[last].append(node.get(arr[last][0]))
    arr[last][1] = arr[last-1][1]
    return arr

def callback(msg):
    try:
        # shortest_dist = addCoordinates(dijkstra(graph, msg.startNode, msg.endNode, burgerDontGo if msg.type == 'burger' else None))
        # print(msg.startNode, msg.endNode)
        shortest_dist = addCoordinates(dijkstra(graph, msg.startNode, msg.endNode))
        # print(shortest_dist)
        loc = Locations()
        for i in range(len(shortest_dist)):
            coord = Coordinate()
            coord.QR = shortest_dist[i][0]
            coord.x = shortest_dist[i][2][1]
            coord.y = shortest_dist[i][2][0]
            coord.deg = shortest_dist[i][1]
            loc.location.append(coord)
        pub.publish(loc)
    except:
        print('error')

def callbackCT(msg):
    lenght = PathLen()
    try:
        # if msg.type == "burger":
        #     shortest_dist = addCoordinates(dijkstra(graph, msg.startNode, msg.endNode, burgerDontGo))
        # else:
        #     shortest_dist = addCoordinates(dijkstra(graph, msg.startNode, msg.endNode))
        # print(msg.startNode, msg.endNode)
        shortest_dist = addCoordinates(dijkstra(graph, msg.startNode, msg.endNode))
        lenght = len(shortest_dist)
    except:
        print(msg.startNode, msg.endNode,'srv_error')
        lenght = -1
    return lenght

if __name__ == '__main__':
    rospy.init_node('path_pub')
    try:
        pub = rospy.Publisher('path', Locations, queue_size=1)
        sub = rospy.Subscriber('move', Move, callback)
        lenSrv = rospy.Service('/min_len', PathLen, callbackCT)
        rospy.spin()
    except:
        print('qwrgtsfgsdfg')
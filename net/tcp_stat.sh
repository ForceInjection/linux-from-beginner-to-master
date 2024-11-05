#!/bin/bash

echo "监听的 TCP 端口及其对应的 IP、PID、进程名称、建立的连接数量及收发报文量："

# 打印标题行，并设置列宽
printf "%-40s %-10s %-10s %-20s %-20s %-40s\n" "IP" "Port" "PID" "进程名称" "连接数" "收发报文量"

# 解析 netstat 输出并格式化列宽
netstat -tnlp | awk '/^tcp/ {print $4, $7}' | while read -r line; do
    address=$(echo $line | awk '{print $1}')
    pid_info=$(echo $line | awk '{print $2}')
    
    # 判断是 IPv4, IPv6 地址，或监听所有接口 (:: 或 0.0.0.0)
    if [[ $address == :::* ]]; then
        ip=":::"
        port="${address##:::}"
    elif [[ $address == *:*:* ]]; then
        # IPv6 地址：提取格式 [IPv6]:Port
        ip=$(echo $address | sed -E 's/^\[(.*)\]:[0-9]+$/\1/')
        port=$(echo $address | sed -E 's/^.*\]:([0-9]+)$/\1/')
    else
        # IPv4 地址或 0.0.0.0:Port
        ip=$(echo $address | cut -d':' -f1)
        port=$(echo $address | cut -d':' -f2)
        [[ $ip == "0.0.0.0" ]] && ip="0.0.0.0"
    fi

    pid=$(echo $pid_info | cut -d'/' -f1)
    if [[ -n "$pid" && "$pid" =~ ^[0-9]+$ ]]; then
        pname=$(ps -p $pid -o comm=)
        connections=$(netstat -tn | awk -v port=$port '/^tcp/ && $4 ~ ":"port"$" {count++} END {print count+0}')

        # 获取收发报文量
        traffic=$(ss -tin "sport = :$port" | awk '
            /bytes_acked/ { 
                for (i=1; i<=NF; i++) {
                    if ($i ~ /bytes_acked/) { tx += substr($i, index($i, ":")+1) }
                    if ($i ~ /bytes_received/) { rx += substr($i, index($i, ":")+1) }
                }
            }
            END {printf "%.2fMB/%.2fMB\n", rx/1024/1024, tx/1024/1024}
        ')
        # 输出每一行数据
        printf "%-40s %-10s %-10s %-20s %-10s %-20s\n" "$ip" "$port" "$pid" "$pname" "$connections" "$traffic"
    else
        printf "%-40s %-10s %-10s %-20s %-10s %-20s\n" "$ip" "$port" "-" "-" "-" "-"
    fi
# 对输出按 IP 和数值端口排序
done | sort -k1,1 -k2,2n
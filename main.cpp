#include <iostream>
#include <QtCore>


QString convert_rcl(QString line);

int main(int argc, char **argv)
{
    if (argc!=3) {
        std::cout<<"Usage:"<<argv[0]<<" qucs_netlist.net spice_netlist.cir\n";
        return 1;
    }

    QFile qnet_file(argv[1]);
    QFile qsp_file(argv[2]);


    QRegExp res_pattern("^[ \t]*R:[A-Za-z]+.*");
    QRegExp cap_pattern("^[ \t]*C:[A-Za-z]+.*");
    QRegExp ind_pattern("^[ \t]*L:[A-Za-z]+.*");
    QRegExp diode_pattern("^[ \t]*Diode:[A-Za-z]+.*");
    QRegExp mosfet_pattern("^[ \t]*MOSFET:[A-Za-z]+.*");

    QString s="";

    if (qnet_file.open(QIODevice::ReadOnly)) {
        QTextStream qucs_netlist(&qnet_file);
        while (!qucs_netlist.atEnd()) {
            QString line = qucs_netlist.readLine();
            if (res_pattern.exactMatch(line)) s += convert_rcl(line);
            if (cap_pattern.exactMatch(line)) s += convert_rcl(line);
        }
        qnet_file.close();
    }

    return 0;
}

QString convert_rcl(QString line)
{
    QString s="";
    QStringList lst = line.split(" ",QString::SkipEmptyParts);
    QString s1 = lst.takeFirst();
    s += s1.remove(':');
    s += " " + lst.takeFirst();
    s += " " + lst.takeFirst() + " ";
    s1 = lst.takeFirst().remove("\"");
    int idx = s1.indexOf('=');
    s += s1.right(s1.count()-idx-1);
    qDebug()<<s;
    return s;
}


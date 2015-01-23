#include <iostream>
#include <QtCore>


QString convert_rcl(QString line);
QString convert_header(QString line);
QString convert_diode(QString line);
QString convert_mosfet(QString line);
QString convert_cccs(QString line);
QString convert_ccvs(QString line);
QString convert_ccs(QString line, bool voltage);
QString convert_vccs(QString line);
QString convert_vcvs(QString line);
QString convert_vcs(QString line, bool voltage);

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
    QRegExp ccvs_pattern("^[ \t]*CCVS:[A-Za-z]+.*");
    QRegExp cccs_pattern("^[ \t]*CCCS:[A-Za-z]+.*");
    QRegExp vcvs_pattern("^[ \t]*VCVS:[A-Za-z]+.*");
    QRegExp vccs_pattern("^[ \t]*VCCS:[A-Za-z]+.*");
    QRegExp subckt_head_pattern("^[ \t]*\\.Def:[A-Za-z]+.*");
    QRegExp ends_pattern("^[ \t]*\\.Def:End[ \t]*$");

    QString s="";

    if (qnet_file.open(QIODevice::ReadOnly)) {
        QTextStream qucs_netlist(&qnet_file);
        while (!qucs_netlist.atEnd()) {
            QString line = qucs_netlist.readLine();
            if (subckt_head_pattern.exactMatch(line)) {
                if (ends_pattern.exactMatch(line)) s += ".ENDS\n";
                else s += convert_header(line);
            }
            if (res_pattern.exactMatch(line)) s += convert_rcl(line);
            if (cap_pattern.exactMatch(line)) s += convert_rcl(line);
            if (ind_pattern.exactMatch(line)) s += convert_rcl(line);
            if (diode_pattern.exactMatch(line)) s += convert_diode(line);
            if (mosfet_pattern.exactMatch(line)) s += convert_mosfet(line);
            if (vccs_pattern.exactMatch(line)) s += convert_vccs(line);
            if (vcvs_pattern.exactMatch(line)) s += convert_vcvs(line);
            if (cccs_pattern.exactMatch(line)) s+= convert_cccs(line);
        }
        qnet_file.close();
    }

    qDebug()<<s;

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
    s += "\n";
    return s;
}

QString convert_header(QString line)
{
    QString s = line;
    s.replace(".Def:",".SUBCKT ");
    s += "\n";
    return s;
}

QString convert_diode(QString line)
{
    QString s="";
    QStringList lst = line.split(" ",QString::SkipEmptyParts);
    QString name = lst.takeFirst();
    int idx = name.indexOf(':');
    name =  name.right(name.count()-idx-1); // name
    QString K = lst.takeFirst();
    QString A = lst.takeFirst();
    s += QString("D%1 %2 %3 DMOD_%4 \n").arg(name).arg(A).arg(K).arg(name);
    QString mod_params = lst.join(" ");
    mod_params.remove('\"');
    s += QString(".MODEL DMOD_%1 D(%2) \n").arg(name).arg(mod_params);
    return s;
}

QString convert_mosfet(QString line)
{
    QString s="";
    QStringList lst = line.split(" ",QString::SkipEmptyParts);
    QString name = lst.takeFirst();
    int idx = name.indexOf(':');
    name =  name.right(name.count()-idx-1); // name
    QString G = lst.takeFirst();
    QString D = lst.takeFirst();
    QString S = lst.takeFirst();
    QString Sub = lst.takeFirst();
    QString L = "";
    QString W = "";
    QString Typ = "NMOS";
    QStringList par_lst;
    par_lst.clear();
    for(int i=0;i<lst.count();i++) {
        QString s1 = lst.at(i);
        if (s1.startsWith("L=\"")) {
            s1.remove('\"');
            L = s1;
        } else if (s1.startsWith("W=\"")) {
            s1.remove('\"');
            W = s1;
        } else if (s1.startsWith("Type=\"nfet\"")) {
            Typ = "NMOS";
        } else if (s1.startsWith("Type=\"pfet\"")) {
            Typ = "PMOS";
        } else {
            par_lst.append(s1); // usual parameter
        }
    }
    s += QString("M%1 %2 %3 %4 %5 MMOD_%6 %7 %8 \n").arg(name).arg(D).arg(G).arg(S).arg(Sub)
            .arg(name).arg(L).arg(W);
    QString mod_params = par_lst.join(" ");
    mod_params.remove('\"');
    s += QString(".MODEL MMOD_%1 %2(%3) \n").arg(name).arg(Typ).arg(mod_params);
    return s;
}

QString convert_cccs(QString line)
{
    return convert_ccs(line,false);
}

QString convert_ccvs(QString line)
{
    return convert_ccs(line,true);
}

QString convert_ccs(QString line, bool voltage)
{
    QStringList lst = line.split(" ",QString::SkipEmptyParts);
    QString name = lst.takeFirst();
    int idx = name.indexOf(':');
    name =  name.right(name.count()-idx-1); // name

    QString nod0 = lst.takeFirst();
    QString nod1 = lst.takeFirst();
    QString nod2 = lst.takeFirst();
    QString nod3 = lst.takeFirst();
    QString s1 = lst.takeFirst().remove("\"");
    idx = s1.indexOf('=');
    QString val = s1.right(s1.count()-idx-1);
    QString s;
    if (voltage) s="H";
    else s="F";
    s += QString("%1 %2 %3 V%4 %5\n").arg(name).arg(nod1).arg(nod2).arg(name).arg(val); // output source nodes
    s += QString("V%1 %2 %3 DC 0\n").arg(name).arg(nod0).arg(nod3);   // controlling 0V source
    return s;
}

QString convert_vccs(QString line)
{
    return convert_vcs(line,false);
}

QString convert_vcvs(QString line)
{
    return convert_vcs(line,true);
}

QString convert_vcs(QString line,bool voltage)
{
    QStringList lst = line.split(" ",QString::SkipEmptyParts);
    QString name = lst.takeFirst();
    int idx = name.indexOf(':');
    name =  name.right(name.count()-idx-1); // name

    QString nod0 = lst.takeFirst();
    QString nod1 = lst.takeFirst();
    QString nod2 = lst.takeFirst();
    QString nod3 = lst.takeFirst();
    QString s1 = lst.takeFirst().remove("\"");
    idx = s1.indexOf('=');
    QString val = s1.right(s1.count()-idx-1);

    QString s;
    if (voltage) s="E";
    else s="G";
    s += QString("%1 %2 %3 %4 %5 %6\n").arg(name).arg(nod1).arg(nod2).arg(nod0).arg(nod3).arg(val);
    return s;
}
